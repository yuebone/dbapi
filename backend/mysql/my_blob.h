//
// Created by bon on 15-4-26.
//

#ifndef DBAPI_MY_BLOB_H
#define DBAPI_MY_BLOB_H

#include "my_def.h"
#include "type/db_blob.h"
#include "utils/noncopyable.h"
#include "y_smart_ptr/y_smart_ptr.h"

namespace dbapi
{

	namespace my
	{
		typedef details::in_raw_buf_blob my_blob;


		/*
		class my_stmt_result_set;

		
		class my_blob :public details::in_blob_backend
		{
		public:
			static unsigned long MAX_BLOB_FETCH_SIZE ;

			my_blob(my_stmt_result_set* rs, int i, std::size_t len) :
				rs_(rs), pos_(i), length_(len),has_read_(false){}

			my_blob() :rs_(0), pos_(-1), length_(0){}

			virtual ~my_blob(){}

			virtual std::size_t length()const
			{
				return length_;
			}

			virtual std::size_t read(std::size_t offset, char *out_buf, std::size_t to_read);

			virtual raw_buf to_raw_buf(bool* convert_success);

		private:
			std::size_t length_;
			int pos_;

			my_stmt_result_set* rs_;

			typedef y::unique_ptr<char[]> buf_t;
			buf_t additional_buf_;

			bool has_read_;

			NONCOPYABLE(my_blob);
		};
	*/
	}
}

#endif