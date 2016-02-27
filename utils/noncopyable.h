//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_NONCOPYABLE_H
#define DBAPI_NONCOPYABLE_H

namespace dbapi
{

    class noncopyable
    {
    public:
        noncopyable() { }

    private:
        noncopyable(const noncopyable &);

        noncopyable &operator=(const noncopyable &);
    };


#define NONCOPYABLE(class_name) \
    class_name(const class_name &);   \
    class_name &operator=(const class_name &)
}

#endif //DBAPI_NONCOPYABLE_H
