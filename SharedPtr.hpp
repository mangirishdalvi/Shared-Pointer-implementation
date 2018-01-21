#include <atomic>
//=============================HELPER===============================
struct CountHelper {
    std::atomic<std::size_t> count;
    CountHelper():count(0) {};
    virtual ~CountHelper() {}
    void operator++(int);
    void operator--(int);
};

template<typename U>
struct DestructorHelper: public CountHelper {
    U *ptr;
    DestructorHelper():ptr(nullptr) {}
    ~DestructorHelper()
    {
        delete ptr;
        ptr=nullptr;
    }
};

void CountHelper::operator++(int)
{
    count++;
}

void CountHelper::operator--(int)
{
    (count.load()==0)?true:count--;
}

template <typename T>
class SharedPtr {
    T *ptr;
    CountHelper *_helper;
public:
//========================Constructors, Assignment Operators, and Destructor=======================
    SharedPtr():ptr(nullptr),_helper(nullptr) {}
    template <typename U>
    explicit SharedPtr(U *);
    SharedPtr(const SharedPtr &);
    template <typename U>
    SharedPtr(const SharedPtr<U> &);
    SharedPtr(SharedPtr &&);
    template <typename U>
    SharedPtr(SharedPtr<U> &&);
    SharedPtr &operator=(const SharedPtr &);
    template <typename U>
    SharedPtr<T> &operator=(const SharedPtr<U> &);
    SharedPtr &operator=(SharedPtr &&p);
    template <typename U>
    SharedPtr &operator=(SharedPtr<U> &&p);
    ~SharedPtr();

//===================================Observers===================================
    T *get() const;
    T &operator*() const;
    T *operator->() const;
    explicit operator bool() const
    {
        return (ptr!=nullptr);
    }

//==================================Modifiers====================================
    void reset();
    template <typename U> void reset(U *p);



//==============================Friend Classes and Functions=====================
    template <typename U>
    friend class SharedPtr;
    template <typename T1,typename U>
    friend SharedPtr<T1> dynamic_pointer_cast(const SharedPtr<U> &);

private:
    std::size_t use_count();

};

//========================Constructors, Assignment Operators, and Destructor=======================
template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(U *p)
{
    ptr=static_cast<T*>(p);
    _helper=new DestructorHelper<U>;
    static_cast<DestructorHelper<U> *>(_helper)->ptr=p;
    (*_helper)++;
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr<T> &p)
{
    _helper=p._helper;
    ptr=p.ptr;
    (_helper!=nullptr)?(*_helper)++:(void)*_helper;
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(const SharedPtr<U> &p)
{
    _helper=(DestructorHelper<T> *)(p._helper);
    ptr=static_cast<T*>(p.ptr);
    if(_helper!=nullptr) {
        static_cast<DestructorHelper<U> *>(_helper)->ptr=p.ptr;
        (*_helper)++;
    }
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr<T> &&p)
{
    _helper=p._helper;
    ptr=static_cast<T*>(p.ptr);
    p._helper=nullptr;
    p.ptr=nullptr;
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(SharedPtr<U> &&p)
{
    _helper=(DestructorHelper<T> *)(p._helper);
    static_cast<DestructorHelper<U> *>(_helper)->ptr=p.ptr;
    ptr=static_cast<T*>(p.ptr);
    p._helper=nullptr;
    p.ptr=nullptr;
}

template <typename T>
SharedPtr<T>::~SharedPtr()
{
    if(_helper!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
        }
    }
}

template <typename T>
SharedPtr<T> &SharedPtr<T>::operator=(const SharedPtr<T> &s)
{
    if(s==(*this))
        return *this;

    if(ptr!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
            // delete ptr;
        }
    }

    _helper=s._helper;
    ptr=s.ptr;
    if(_helper!=nullptr)
        (*_helper)++;

    return *this;
}

template <typename T>
template <typename U>
SharedPtr<T> &SharedPtr<T>::operator=(const SharedPtr<U> &s)
{
    if(s==(*this))
        return *this;

    if(ptr!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
            // delete ptr;
        }
    }

    _helper=(DestructorHelper<T> *)(s._helper);
    ptr=static_cast<T*>(s.ptr);
    if(_helper!=nullptr) {
        static_cast<DestructorHelper<U> *>(_helper)->ptr=s.ptr;
        (*_helper)++;
    }

    return *this;
}

template <typename T>
SharedPtr<T> &SharedPtr<T>::operator=(SharedPtr<T> &&p)
{

    if(ptr!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
            // delete ptr;
        }
    }

    ptr=p.ptr;
    _helper=p._helper;
    p._helper=nullptr;
    p.ptr=nullptr;

    return *this;
}

template <typename T>
template <typename U>
SharedPtr<T> &SharedPtr<T>::operator=(SharedPtr<U> &&p)
{
    if(ptr!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
            // delete ptr;
        }
    }

    _helper=(DestructorHelper<T> *)(p._helper);
    static_cast<DestructorHelper<U> *>(_helper)->ptr=p.ptr;
    ptr=static_cast<T*>(p.ptr);
    p._helper=nullptr;
    p.ptr=nullptr;

    return *this;
}

//===================================Observers===================================
template <typename T>
T *SharedPtr<T>::get() const
{
    return ptr;
}

template <typename T>
T &SharedPtr<T>::operator*() const
{
    return *ptr;
}

template <typename T>
T *SharedPtr<T>::operator->() const
{
    return ptr;
}

//==================================Modifiers====================================
template <typename T>
void SharedPtr<T>::reset()
{
    if(_helper!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
            // delete ptr;
        }
        _helper=nullptr;
        ptr=nullptr;
    }

}

template <typename T>
template <typename U>
void SharedPtr<T>::reset(U *p)
{
    if(_helper!=nullptr) {
        (*_helper)--;
        if(_helper->count.load()==0) {
            delete _helper;
            // delete ptr;
        }
    }
    _helper=new DestructorHelper<U>;
    if(_helper!=nullptr) {
        static_cast<DestructorHelper<U> *>(_helper)->ptr=p;
        (*_helper)++;
    }
    ptr=static_cast<T*>(p);
}

//======================================Free Functions==================================
template <typename T1, typename T2>
bool operator==(const SharedPtr<T1> &s1, const SharedPtr<T2> &s2)
{
    return (s1.get()==s2.get());
}

template <typename T1, typename T2>
bool operator!=(const SharedPtr<T1>&s1, const SharedPtr<T2> &s2)
{
    return !(s1==s2);
}

template <typename T>
bool operator==(const SharedPtr<T> &s1, std::nullptr_t)
{
    return (s1.get()==nullptr);
}

template <typename T>
bool operator==(std::nullptr_t, const SharedPtr<T> &s1)
{
    return (s1.get()==nullptr);
}

template <typename T>
bool operator!=(const SharedPtr<T> &s1, std::nullptr_t)
{
    return !(s1==nullptr);
}

template <typename T>
bool operator!=(std::nullptr_t, const SharedPtr<T> &s1)
{
    return !(s1==nullptr);
}

template <typename T, typename U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U> &sp)
{
    return SharedPtr<T>(sp);
}

template <typename T, typename U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &sp)
{
    T *ptr=dynamic_cast<T*>(sp.get());
    SharedPtr<T> res;
    res.ptr=ptr;
    if(res.ptr!=nullptr) {
        res._helper=sp._helper;
        (*(res._helper))++;
    }
    return res;

}


//====================================Helper Function==============================
template <typename T>
std::size_t SharedPtr<T>::use_count()
{
    return _helper->count.load();
}

