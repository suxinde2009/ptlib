/*
 * configure.cxx
 *
 * Build options generated by the configure script.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: configure.cpp,v $
 * Revision 1.1  2003/04/16 08:00:19  robertj
 * Windoes psuedo autoconf support
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stdlib.h>
#include <windows.h>


using namespace std;


class Feature
{
  public:
    Feature(
      const char * dispName,
      const char * defName,
      const char * defValue,
      const char * dirName = "",
      const char * incName = "",
      const char * incText = "",
      const char * dir1 = "",
      const char * dir2 = ""
    )
    : displayName(dispName),
      defineName(defName),
      defineValue(defValue),
      directoryName(dirName),
      includeName(incName),
      includeText(incText)
    {
      if (includeName[0] == '\0')
        found = true;
      else {
        found = false;
        if (!Locate(dir1))
          Locate(dir2);
      }
    }

    virtual void Adjust(string & line)
    {
      if (found && line.find("#undef") != string::npos && line.find(defineName) != string::npos)
        line = "#define " + defineName + ' ' + defineValue;

      if (directoryName[0] != '\0') {
        int pos = line.find(directoryName);
        if (pos != string::npos)
          line.replace(pos, directoryName.length(), directory);
      }
    }

    virtual bool Locate(const char * testDir)
    {
      if (found)
        return true;

      string testDirectory = testDir;
      if (testDirectory[testDirectory.length()-1] != '\\')
        testDirectory += '\\';

      string filename = testDirectory + includeName;
      ifstream file(filename.c_str(), ios::in);
      if (!file.is_open())
        return false;

      if (includeText[0] == '\0')
        found = true;
      else {
        while (file.good()) {
          string line;
          getline(file, line);
          if (line.find(includeText) != string::npos) {
            found = true;
            break;
          }
        }
      }

      if (!found)
        return false;

      char buf[_MAX_PATH];
      _fullpath(buf, testDirectory.c_str(), _MAX_PATH);
      directory = buf;

      cout << "Located " << displayName << " at " << directory << endl;

      int pos;
      while ((pos = directory.find('\\')) != string::npos)
        directory[pos] = '/';
      pos = directory.length()-1;
      if (directory[pos] == '/')
        directory.erase(pos);

      return true;
    }

    bool IsFound() const { return found; }

    friend ostream & operator<<(ostream & stream, const Feature & feature)
    {
      stream << feature.displayName << ' ';
      if (feature.found)
        stream << "enabled";
      else
        stream << "disabled";
      return stream;
    }

  protected:
    string displayName;
    string defineName;
    string defineValue;
    string directoryName;
    string includeName;
    string includeText;

    bool found;
    string directory;
};


list<Feature> features;


bool TreeWalk(const string & directory)
{
  bool foundAll = false;

  string wildcard = directory;
  wildcard += "*.*";

  WIN32_FIND_DATA fileinfo;
  HANDLE hFindFile = FindFirstFile(wildcard.c_str(), &fileinfo);
  if (hFindFile != NULL) {
    do {
      string subdir = directory;
      subdir += fileinfo.cFileName;

      if ((fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                                            fileinfo.cFileName[0] != '.') {
        subdir += '\\';

        foundAll = true;
        list<Feature>::iterator feature;
        for (feature = features.begin(); feature != features.end(); feature++) {
          if (!feature->Locate(subdir.c_str()))
            foundAll = false;
        }

        if (foundAll)
          break;

        TreeWalk(subdir);
      }
    } while (FindNextFile(hFindFile, &fileinfo));

    FindClose(hFindFile);
  }

  return foundAll;
}


int main(int /*argc*/, char* /*argv[]*/)
{
  features.push_back(Feature("Byte Order",   "PBYTE_ORDER",      "PLITTLE_ENDIAN"));
  features.push_back(Feature("DLL Support",  "P_DYNALINK",       "1"));
  features.push_back(Feature("Semaphores",   "P_HAS_SEMAPHORES", "1"));

  features.push_back(Feature("IPv6",         "P_HAS_IPV6",       "1", "@IPV6_DIR@",  "ws2tcpip.h",            "sin6_scope_id", "\\Program Files\\Microsoft Visual Studio\\VC98\\Include\\", "\\Program Files\\Microsoft SDK\\"));
  features.push_back(Feature("OpenSSL",      "P_SSL",            "1", "@SSL_DIR@",   "inc32\\openssl\\ssl.h", "",              "..\\openssl\\"));
  features.push_back(Feature("Expat XML",    "P_EXPAT",          "1", "@EXPAT_DIR@", "lib\\expat.h",          "",              "..\\expat\\"));
  features.push_back(Feature("OpenLDAP",     "P_LDAP",           "1", "@LDAP_DIR@",  "include\\ldap.h",       "OpenLDAP",      "..\\openldap\\"));
  features.push_back(Feature("Speech API",   "P_SAPI",           "1", "",            "include\\sphelper.h",   "",              "\\Program Files\\Microsoft Speech SDK 5.1\\"));
  features.push_back(Feature("DNS Resolver", "P_DNS",            "1", "@DNS_DIR@",   "include\\windns.h",     "",              "\\Program Files\\Microsoft SDK\\"));

  ifstream in("include/ptbuildopts.h.in", ios::in);
  if (!in.is_open()) {
    cerr << "Could not open ptbuildopts.h.in" << endl;
    return 1;
  }

  ofstream out("include/ptbuildopts.h", ios::out);
  if (!out.is_open()) {
    cerr << "Could not open ptbuildopts.h" << endl;
    return 1;
  }

  bool foundAll = true;
  list<Feature>::iterator feature;
  for (feature = features.begin(); feature != features.end(); feature++) {
    if (!feature->IsFound())
      foundAll = false;
  }

  if (!foundAll) {
    // Do search of entire system
    char drives[100];
    if (!GetLogicalDriveStrings(sizeof(drives), drives))
      strcpy(drives, "C:\\");

    const char * drive = drives;
    while (*drive != '\0') {
      if (GetDriveType(drive) == DRIVE_FIXED) {
        cout << "Searching " << drive << endl;
        if (TreeWalk(drive))
          break;
      }
      drive += strlen(drive)+1;
    }
  }

  while (in.good()) {
    string line;
    getline(in, line);

    for (feature = features.begin(); feature != features.end(); feature++)
      feature->Adjust(line);

    out << line << '\n';
  }

  cout << "Configuration completed:\n";
  for (feature = features.begin(); feature != features.end(); feature++)
    cout << "  " << *feature << '\n';
  cout << endl;

  return 0;
}
