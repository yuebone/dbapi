#include <new>
#include "my_blob.h"
#include "my_backend.h"

namespace dbapi
{
	namespace my
	{
		/*
		unsigned long my_blob::MAX_BLOB_FETCH_SIZE = 1024 * 1024;

		std::size_t my_blob::read(std::size_t offset, char *out_buf, std::size_t to_read)
		{
			if (additional_buf_)
			{
				std::size_t read_size_ = (length_ - offset) > to_read ? to_read : (length_ - offset);
				memcpy(out_buf, additional_buf_.get() + offset, read_size_);
				return read_size_;
			}

			std::size_t left = to_read;
			void* im_buf = rs_->curr_fields_[pos_].buf.get();
			std::size_t off = 0;

			while (left)
			{
				std::size_t read_size = left > MAX_BLOB_FETCH_SIZE ? MAX_BLOB_FETCH_SIZE : left;

				if (mysql_stmt_fetch_column(rs_->my_stmt_, rs_->rs_binds_.get(), pos_, off))
					_throw_error(rs_->my_stmt_);
				memcpy(out_buf, im_buf, read_size);

				left -= read_size;
				off += read_size;
			}
			return off;
		}

		raw_buf my_blob::to_raw_buf(bool* convert_success)
		{
			if (length_ <= MAX_BLOB_FETCH_SIZE)
			{
				*convert_success = true;
				return raw_buf((void*)rs_->curr_fields_[pos_].buf.get(),length_);
			}
			if (!additional_buf_)
			{
				char* buf__ = new (std::nothrow) char[length_];
				if (buf__)
				{
					*convert_success = false;
					return raw_buf();
				}


				std::size_t left = length_;
				void* im_buf = rs_->curr_fields_[pos_].buf.get();
				std::size_t off = 0;

				while (left)
				{
					std::size_t read_size = left > MAX_BLOB_FETCH_SIZE ? MAX_BLOB_FETCH_SIZE : left;

					if (mysql_stmt_fetch_column(rs_->my_stmt_, rs_->rs_binds_.get(), pos_, off))
						_throw_error(rs_->my_stmt_);
					memcpy(additional_buf_.get(), im_buf,read_size);
					
					left -= read_size;
					off += read_size;
				}
				
			}

			*convert_success = true;
			return raw_buf(additional_buf_.get(), length_);

		}
		*/
	}
}