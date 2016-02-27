//
// Created by bon on 15-4-20.
//

#ifndef DBAPI_DB_BLOB_H
#define DBAPI_DB_BLOB_H

#include <istream>
#include <ostream>
#include "backend/blob_backend.h"
#include "y_smart_ptr/y_smart_ptr.h"
#include "type_def.h"
#include "type_traits.h"
#include "type_conversion.h"
#include "db_type.h"
#include "dbapi_config.h"

namespace dbapi
{

    /*
     * in_blob reader
     * defined by user,called by in_blob
     * argument :
     *  piece_index : indicates which piece data to read at this call,
     *                -1 indicates the last piece
     *  in_buf      : buffer to read from
     *  buf_len     : len of in_buf
     *  max_buf_len : since buf_len may be variable-length,
     *                max_buf_len is the max of buf_len,0 indicates unknown
     * */
    typedef void (*blob_reader)//not used yet , FIX ME
            (int piece_index,
            const char* in_buf,
            std::size_t buf_len,
            std::size_t max_buf_len);

    /*
     * out_blob writer
     * defined by user,called by the backend,where using out_blob
     * argument :
     *  piece_index : indicates which piece data to write at this call,
     *                -1 indicates the last piece
     *  in_buf      : buffer to write into
     *  buf_len     : len of in_buf
     * return:  actually wrote length at this call
     * */
    typedef std::size_t (*blob_writer)
            (int &piece_index,
             char* out_buf,
             std::size_t buf_len);


	/*
	in_blob
	*/
    class DBAPI_DECL in_blob
    {
    public:
        typedef details::user_type_tag type_family;

        enum {type_type=type_in_blob};


        enum {Chunk=1024*1024/*1 M*/};

		in_blob(details::in_blob_backend* blob_be) :
                blob_be_(blob_be){}

		in_blob() :blob_be_(){}

		details::in_blob_backend* get_blob_backend()const
        { return blob_be_.get();}

        std::size_t length()const
        { return blob_be_->length();}

        std::size_t read(std::size_t offset,char *out_buf,std::size_t to_read)
        { return blob_be_->read(offset,out_buf,to_read);}

        std::size_t read(std::ostream &os,std::size_t offset=0,std::size_t chunk_size=Chunk);

        template <class T>
        inline T to()const
        {
            raw_buf r=to<raw_buf>();
            return details::convert_to<raw_buf,T>(r);
        }

		void set(details::in_blob_backend* blob_be)
		{
			blob_be_.reset(blob_be);
		}

    private:
        typedef y::shared_ptr<details::in_blob_backend> blob_be_ptr;
        blob_be_ptr blob_be_;

    };

	template <>
	inline raw_buf in_blob::to<raw_buf>()const
	{
		bool successful = false;
		raw_buf r = blob_be_->to_raw_buf(&successful);
		if (successful)
			return r;
		else
		{
			details::dbapi_type_conversion_error();
			return raw_buf();//without returning will cause warning 
		}
	}


	namespace details
	{

		class in_raw_buf_blob :public in_blob_backend
		{
		public:
			in_raw_buf_blob(const raw_buf& r = raw_buf()) :raw_buf_(r){}

			virtual std::size_t length()const
			{
				return raw_buf_.get_size();
			}

			virtual std::size_t read(std::size_t offset, char *out_buf, std::size_t to_read);

			virtual raw_buf to_raw_buf(bool* convert_success/*out*/)
			{
				*convert_success = true;
				return raw_buf_;
			}

			raw_buf raw_buf_;
		};



        template <>
        inline in_blob convert_to<raw_buf,in_blob>(const raw_buf& from)
        {
            details::in_blob_backend* b = new details::in_raw_buf_blob(from);
            return in_blob(b);
        }

        template <>
        inline in_blob convert_to<char_str, in_blob>(const char_str& from)
        {
            details::in_blob_backend* b =
                    new details::in_raw_buf_blob(raw_buf((void*)from.ptr,from.size));
            return in_blob(b);
        }
	}






    class DBAPI_DECL out_blob
    {
        struct blob_buf
        {
            void* ptr;
            std::size_t size;
        };

    public:
        enum out_blob_type
        {
            from_stream,
            from_writer,
            from_buf,
            from_null
        };

        typedef details::user_type_tag type_family;

        enum {type_type=type_out_blob};

        out_blob():from_type_(from_null),in_blob_(0){}

        out_blob(std::istream* is):
                is_(is),from_type_(from_stream),in_blob_(0){}

        out_blob(blob_writer writer):
                writer_(writer),from_type_(from_writer),in_blob_(0){}

        out_blob(void* buf,std::size_t size):
                from_type_(from_buf),in_blob_(0)
        {
            buf_.ptr=buf;
            buf_.size=size;
        }


        std::istream* get_stream()const
        {
            if(from_type_==from_stream)
                return is_;
            else
                return 0;
        }

        blob_writer get_writer()const
        {
            if(from_type_==from_writer)
                return writer_;
            else
                return 0;
        }

        raw_buf get_buf()const
        {
            if(from_type_==from_buf)
                return raw_buf(buf_.ptr,buf_.size);
            else
                return raw_buf();
        }

        out_blob_type get_from_type()const
        {
            return from_type_;
        }

        template <class T>
        inline T to()const
        {
            details::dbapi_type_conversion_error();
            return T();
        }

        in_blob get_associate_in_blob()const
        {
            return *in_blob_;
        }

        void set_associate_in_blob(in_blob* in)
        {
            in_blob_=in;
        }

    private:
        union
        {
            std::istream *is_;
            blob_writer writer_;
            blob_buf buf_;
        };

        out_blob_type from_type_;

        //may not used
        in_blob* in_blob_;
    };
}

#endif //DBAPI_DB_BLOB_H
