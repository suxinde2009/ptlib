/*
 * safecoll.h
 *
 * Thread safe collection classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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
 * $Log: safecoll.h,v $
 * Revision 1.1  2002/05/01 04:16:43  robertj
 * Added thread safe collection classes.
 *
 */
 
#ifndef _SAFE_COLLECTION_H
#define _SAFE_COLLECTION_H

#ifdef __GNUC__
#pragma interface
#endif


/** This class defines a thread-safe object in a collection.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  The act of adding a new object is simple and only requires locking the
  collection itself during the add.

  Locating an object is more complicated. The obvious lock on the collection
  is made for the initial search. But we wish to have the full collection lock
  for as short a period as possible (for performance reasons) so we lock the
  individual object and release the lock on the collection.

  A simple mutex on the object however is very dangerous as it can be (and
  should be able to be!) locked from other threads independently of the
  collection. If one of these threads subsequently needs to get at the
  collection (eg it wants to remove the object) then we will have a deadlock.
  Also, to avoid a race condition with the object begin deleted, the objects
  lock must be made while the collection lock is set. The performance gains
  are then lost as if something has the object locked for a long time, then
  another object wanting that object will actually lock the collection for a
  long time as well.

  So, an object has 4 states: unused, referenced, reading & writing. With the
  additional rider of "being removed". This flag prevents new locks from being
  acquired and waits for all locks to be relinquished before removing the
  object from the system. This prevents numerous race conditions and accesses
  to deleted objects.

  The "unused" state indicates the object exists in the collection but no
  threads anywhere is using it. It may be moved to any state by any thread
  while in this state. An object cannot be deleted (ie memory deallocated)
  until it is unused.

  The "referenced" state indicates that a thread has a reference (eg pointer)
  to the object and it should not be deleted. It may be locked for reading or
  writing at any time thereafter.

  The "reading" state is a form of lock that indicates that a thread is
  reading from the object but not writing. Multiple threads can obtain a read
  lock. Note the read lock has an implicit "reference" state in it.

  The "writing" state is a form of lock where the data in the object may
  be changed. It can only be obtained exclusively and if there are no read
  locks present. Again there is an implicit reference state in this lock.

  Note that threads going to the "referenced" state may do so regardless of
  the read or write locks present.

  Access to safe objects (especially when in a safe collection) is recommended
  to by the PSafePtr<> class which will manage reference counting and the
  automatic unlocking of objects ones the pointer goes out of scope. It may
  also be used to lock each object of a collection in turn.

  The enumeration
 */
class PSafeObject : public PObject
{
    PCLASSINFO(PSafeObject, PObject);
  public:
  /**@name Construction */
  //@{
    /**Create a thread safe object.
     */
    PSafeObject();
  //@}

  /**@name Operations */
  //@{
    /**Increment the reference count for object.
       This will guarantee that the object is not deleted (ie memory
       deallocated) as the caller thread is using the object, but not
       necessarily at this time locking it.

       If the function returns FALSE, then the object has been flagged for
       deletion and the calling thread should immediately cease using the
       object.

       A typical use of this would be when an entity (eg a thread) has a
       pointer to the object but is not currenty accessing the objects data.
       The LockXXX functions may be called independetly of the reference
       system and the pointer beiong used for the LockXXX call is guaranteed
       to be usable.

       It is recommended that the PSafePtr<> class is used to manage this
       rather than the application calling this function directly.
      */
    BOOL SafeReference();

    /**Decrement the reference count for object.
       This indicates that the thread no longer has anything to do with the
       object and it may be deleted (ie memory deallocated).

       It is recommended that the PSafePtr<> class is used to manage this
       rather than the application calling this function directly.
      */
    void SafeDereference();

    /**Lock the object for Read Only access.
       This will lock the object in read only mode. Multiple threads may lock
       the object read only, but only one thread can lock for read/write.
       Also, no read only threads can be present for the read/write lock to
       occur and no read/write lock can be present for any read only locks to
       occur.

       If the function returns FALSE, then the object has been flagged for
       deletion and the calling thread should immediately cease use of the
       object, possibly executing the SafeDereference() function to remove
       any references it may have acquired.

       It is expected that the caller had already called the SafeReference()
       function (directly or implicitly) before calling this function. It is
       recommended that the PSafePtr<> class is used to automatically manage
       the reference counting and locking of objects.
      */
    BOOL LockReadOnly();

    /**Release the read only lock on an object.
       Unlock the read only mutex that a thread had obtained. Multiple threads
       may lock the object read only, but only one thread can lock for
       read/write. Also, no read only threads can be present for the
       read/write lock to occur and no read/write lock can be present for any
       read only locks to occur.

       It is recommended that the PSafePtr<> class is used to automatically
       manage the reference counting and unlocking of objects.
      */
    void UnlockReadOnly();

    /**Lock the object for Read/Write access.
       This will lock the object in read/write mode. Multiple threads may lock
       the object read only, but only one thread can lock for read/write.
       Also no read only threads can be present for the read/write lock to
       occur and no read/write lock can be present for any read only locks to
       occur.

       If the function returns FALSE, then the object has been flagged for
       deletion and the calling thread should immediately cease use of the
       object, possibly executing the SafeDereference() function to remove
       any references it may have acquired.

       It is expected that the caller had already called the SafeReference()
       function (directly or implicitly) before calling this function. It is
       recommended that the PSafePtr<> class is used to automatically manage
       the reference counting and locking of objects.
      */
    BOOL LockReadWrite();

    /**Release the read/write lock on an object.
       Unlock the read/write mutex that a thread had obtained. Multiple threads
       may lock the object read only, but only one thread can lock for
       read/write. Also, no read only threads can be present for the
       read/write lock to occur and no read/write lock can be present for any
       read only locks to occur.

       It is recommended that the PSafePtr<> class is used to automatically
       manage the reference counting and unlocking of objects.
      */
    void UnlockReadWrite();

    /**Set the removed flag.
       This flags the object as beeing removed but does not physically delete
       the memory being used by it. The SafelyCanBeDeleted() can then be used
       to determine when all references to the object have been released so it
       may be safely deleted.

       This is typically used by the PSafeCollection class and is not expected
       to be used directly by an application.
      */
    void SafeRemove();

    /**Determine if the object can be safely deleted.
       This determines if the object has been flagged for deletion and all
       references to it have been released.

       This is typically used by the PSafeCollection class and is not expected
       to be used directly by an application.
      */
    BOOL SafelyCanBeDeleted();
  //@}

  protected:
    PMutex          safetyMutex;
    unsigned        safeReferenceCount;
    BOOL            safelyBeingRemoved;
    PReadWriteMutex safeInUseFlag;
};


/** This class defines a thread-safe collection of objects.
  This class is a wrapper around a standard PCollection class which allows
  only safe, mutexed, access to the collection.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
class PSafeCollection : public PObject
{
    PCLASSINFO(PSafeCollection, PObject);
  public:
  /**@name Construction */
  //@{
    /**Create a thread safe collection of objects.
       Note the collection is automatically deleted on destruction.
     */
    PSafeCollection(
      PCollection * collection    /// Actual collection of objects
     );

    /**Destroy the thread safe collection.
       The will delete the collection object provided in the constructor.
      */
    ~PSafeCollection();
  //@}

  /**@name Operations */
  //@{
  protected:
    /**Add an object to the collection.
       This uses the PCollection::Append() function to add the object to the
       collection, with full mutual exclusion locking on the collection.
      */
    virtual PINDEX SafeAppend(
      PSafeObject * obj     /// Object to add to collection
    );

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual BOOL SafeRemove(
      PSafeObject * obj   /// Object to remove from collection
    );

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual PSafeObject * SafeRemoveAt(
      PINDEX idx    /// Object index to remove
    );

  public:
    /**Remove all objects in collection.
      */
    virtual void RemoveAll();

    /**Delete any objects that have been removed.
      */
    virtual void DeleteObjectsToBeRemoved();

    /**Start a timer to automatically call DeleteObjectsToBeRemoved().
      */
    virtual void SetAutoDeleteObjects();

    /**Get the current size of the collection.
       Note that use of this function is limited as it is not subject to the
       collections mutual exclusion locking. Thus, it is merely an
       instantaneous snapshot of the state of the collection.
      */
    PINDEX GetSize() const { return collection->GetSize(); }
  //@}

  protected:
    void SafeRemoveObject(PSafeObject * obj);
    PDECLARE_NOTIFIER(PTimer, PSafeCollection, DeleteObjectsTimeout);

    PCollection * collection;
    PAbstractList toBeRemoved;
    PMutex        collectionMutex;
    PTimer        deleteObjectsTimer;

  friend class PSafePtrBase;
};


enum PSafetyMode {
  PSafeReference,
  PSafeReadOnly,
  PSafeReadWrite
};

/** This class defines a base class for thread-safe pointer to an object.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  NOTE: the PSafePtr will allow safe and mutexed access to objects but is not
  thread safe itself! You should not share PSafePtr instances across threads.
  You can assign a PSafePtr to another instance across a thread boundary
  provided it is on a reference and no read only or read/write locks are
  present.

  See the PSafeObject class for more details.
 */
class PSafePtrBase : public PObject
{
    PCLASSINFO(PSafePtrBase, PObject);

  /**@name Construction */
  //@{
  protected:
    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       Note that this version is not associated with a collection so the ++
       and -- operators will not work.
     */
    PSafePtrBase(
      PSafeObject * obj = NULL,         /// Physical object to point to.
      PSafetyMode mode = PSafeReference /// Locking mode for the object
    );

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The idx'th entry of the collection is pointed to by this object. If the
       idx is beyond the size of the collection, the pointer is NULL.
     */
    PSafePtrBase(
      PSafeCollection & safeCollection, /// Collection pointer will enumerate
      PSafetyMode mode,                 /// Locking mode for the object
      PINDEX idx                        /// Index into collection to point to
    );

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The obj parameter is only set if it contained in the collection,
       otherwise the pointer is NULL.
     */
    PSafePtrBase(
      PSafeCollection & safeCollection, /// Collection pointer will enumerate
      PSafetyMode mode,                 /// Locking mode for the object
      PSafeObject * obj                 /// Inital object in collection to point to
    );

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtrBase(
      const PSafePtrBase & enumerator   /// Pointer to copy
    );

  public:
    /**Unlock and dereference the PSafeObject this is pointing to.
      */
    ~PSafePtrBase();
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Compare the pointers.
       Note this is not a value comparison and will only return EqualTo if the
       two PSafePtrBase instances are pointing to the same instance.
      */
    Comparison Compare(
      const PObject & obj   /// Other instance to compare against
    ) const;
  //@}

  /**@name Operations */
  //@{
  public:
    /**Change the locking mode used by this pointer.
      */
    BOOL SetSafetyMode(
      PSafetyMode mode  /// New locking mode
    );

  protected:
    void Assign(const PSafePtrBase & enumerator);
    void Assign(PSafeCollection & safeCollection);
    void Assign(PSafeObject * obj);
    void Assign(PINDEX idx);
    void Next();
    void Previous();

    enum EnterSafetyModeOption {
      WithReference,
      AlreadyReferenced
    };
    BOOL EnterSafetyMode(EnterSafetyModeOption ref);

    enum ExitSafetyModeOption {
      WithDereference,
      NoDereference
    };
    void ExitSafetyMode(ExitSafetyModeOption ref);
  //@}

  protected:
    PSafeCollection * collection;
    PSafeObject     * currentObject;
    PSafetyMode       lockMode;
    PThread         * lockThread;
    unsigned          lockCount;
};


/** This class defines a thread-safe enumeration of object in a collection.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  NOTE: the PSafePtr will allow safe and mutexed access to objects but is not
  thread safe itself! You should not share PSafePtr instances across threads.
  You can assign a PSafePtr to another instance across a thread boundary
  provided it is on a reference and no read only or read/write locks are
  present.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class T> class PSafePtr : public PSafePtrBase
{
    PCLASSINFO(PSafePtr, PSafePtrBase);
  public:
  /**@name Construction */
  //@{
    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       Note that this version is not associated with a collection so the ++
       and -- operators will not work.
     */
    PSafePtr(
      T * obj = NULL,                   /// Physical object to point to.
      PSafetyMode mode = PSafeReference /// Locking mode for the object
    ) : PSafePtrBase(obj, mode) { }

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The idx'th entry of the collection is pointed to by this object. If the
       idx is beyond the size of the collection, the pointer is NULL.
     */
    PSafePtr(
      PSafeCollection & safeCollection,   /// Collection pointer will enumerate
      PSafetyMode mode = PSafeReadWrite,  /// Locking mode for the object
      PINDEX idx = 0                      /// Index into collection to point to
    ) : PSafePtrBase(safeCollection, mode, idx) { }

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The obj parameter is only set if it contained in the collection,
       otherwise the pointer is NULL.
     */
    PSafePtr(
      PSafeCollection & safeCollection, /// Collection pointer will enumerate
      PSafetyMode mode,                 /// Locking mode for the object
      PSafeObject * obj                 /// Inital object in collection to point to
    ) : PSafePtrBase(safeCollection, mode, obj) { }

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtr(
      const PSafePtr & enumerator   /// Pointer to copy
    ) : PSafePtrBase(enumerator) { }

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtr & operator=(const PSafePtr & enumerator)
      {
        Assign(enumerator);
        return *this;
      }

    /**Start an enumerated PSafeObject.
       This will create a read/write locked reference to teh first element in
       the collection.
      */
    PSafePtr & operator=(PSafeCollection & safeCollection)
      {
        Assign(safeCollection);
        return *this;
      }

    /**Set the new pointer to a PSafeObject.
       This will set the pointer to the new object. The old object pointed to
       will be unlocked and dereferenced and the new object referenced and set
       to the same locking mode as the previous pointer value.
     */
    PSafePtr & operator=(T * obj)
      {
        Assign(obj);
        return *this;
      }

    /**Set the new pointer to a collection index.
       This will set the pointer to the new object to the index entry in the
       colelction that the pointer was created with. The old object pointed to
       will be unlocked and dereferenced and the new object referenced and set
       to the same locking mode as the previous pointer value.
     */
    PSafePtr & operator=(PINDEX idx)
      {
        Assign(idx);
        return *this;
      }
  //@}

  /**@name Operations */
  //@{
    /**Return the physical pointer to the object.
      */
    operator T*()    const { return  (T *)currentObject; }

    /**Return the physical pointer to the object.
      */
    T & operator*()  const { return *(T *)currentObject; }

    /**Allow access to the physical object the pointer is pointing to.
      */
    T * operator->() const { return  (T *)currentObject; }

    /**Post-increment the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator++(int)
      {
        T * previous = (T *)currentObject;
        Next();
        return previous;
      }

    /**Pre-increment the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator++()
      {
        Next();
        return (T *)currentObject;
      }

    /**Post-decrement the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator--(int)
      {
        T * previous = (T *)currentObject;
        Previous();
        return previous;
      }

    /**Pre-decrement the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator--()
      {
        Previous();
        return (T *)currentObject;
      }
  //@}
};


/** This class defines a thread-safe list of objects.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Coll, class Base> class PSafeList : public PSafeCollection
{
    PCLASSINFO(PSafeList, PSafeCollection);
  public:
  /**@name Construction */
  //@{
    /**Create a safe list collection wrapper around the real collection.
      */
    PSafeList()
      : PSafeCollection(new Coll)
      { }
  //@}

  /**@name Operations */
  //@{
    /**Add an object to the collection.
       This uses the PCollection::Append() function to add the object to the
       collection, with full mutual exclusion locking on the collection.
      */
    virtual PINDEX Append(
      Base * obj        /// Object to add to safe collection.
    ) {
        return PSafeCollection::SafeAppend(obj);
      }

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual BOOL Remove(
      Base * obj          /// Object to remove from safe collection
    ) {
        return SafeRemove(obj);
      }

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual Base * RemoveAt(
      PINDEX idx     /// Index to remove
    ) {
        return (Base *)SafeRemoveAt(idx);
      }

    /**Get the instance in the collection of the index.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> GetWithLock(
      PINDEX idx,
      PSafetyMode mode = PSafeReadWrite
    ) {
        return PSafePtr<Base>(*this, mode, idx);
      }

    /**Find the instance in the collection of an object with the same value.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> FindWithLock(
      const Base & value,
      PSafetyMode mode = PSafeReadWrite
    ) {
        PWaitAndSignal mutex(collectionMutex);
        return PSafePtr<Base>(*this, mode, collection->GetValuesIndex(value));
      }
  //@}
};


/** This class defines a thread-safe dictionary of objects.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Coll, class Key, class Base> class PSafeDictionary : public PSafeCollection
{
    PCLASSINFO(PSafeDictionary, PSafeCollection);
  public:
  /**@name Construction */
  //@{
    /**Create a safe dictionary wrapper around the real collection.
      */
    PSafeDictionary()
      : PSafeCollection(new Coll) { }
  //@}

  /**@name Operations */
  //@{
    /**Add an object to the collection.
       This uses the PCollection::Append() function to add the object to the
       collection, with full mutual exclusion locking on the collection.
      */
    virtual void SetAt(const Key & key, Base * obj)
      {
        collectionMutex.Wait();
        SafeRemove(((Coll *)collection)->GetAt(key));
        ((Coll *)collection)->SetAt(key, obj);
        collectionMutex.Signal();
      }

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual BOOL RemoveAt(
      const Key & key   /// Key to fund object to delete
    ) {
        PWaitAndSignal mutex(collectionMutex);
        return SafeRemove(((Coll *)collection)->GetAt(key));
      }

    /**Get the instance in the collection of the index.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> GetWithLock(
      PINDEX idx,
      PSafetyMode mode = PSafeReadWrite
    ) {
        return PSafePtr<Base>(*this, mode, idx);
      }

    /**Find the instance in the collection of an object with the same value.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> FindWithLock(
      const Key & key,
      PSafetyMode mode = PSafeReadWrite
    ) {
        PWaitAndSignal mutex(collectionMutex);
        return PSafePtr<Base>(*this, mode, ((Coll *)collection)->GetAt(key));
      }
  //@}
};


#endif // _SAFE_COLLECTION_H


/////////////////////////////////////////////////////////////////////////////
