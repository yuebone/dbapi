//
// Created by bon on 15-5-3.
//
#ifndef DBAPI_DB_METADATA_H
#define DBAPI_DB_METADATA_H

#include "backend/metadata_backend.h"
#include "utils/noncopyable.h"
#include "y_smart_ptr/y_smart_ptr.h"

namespace dbapi
{

	namespace details
	{
		struct empty_column:public column_backend
		{
			const std::string& name() const ;
			const std::string& type_name() const ;
			int native_type_code() const { return -1;}
			int pos() const { return 0;}
			bool required() const { return false;}
			std::size_t size() const { return 0;}
			int precision() const { return 0;}
			int scale() const { return 0;}
		};
		const empty_column empty_column_();

		struct empty_table:public table_backend
		{
			table_type get_table_type() const { return base_table;}
			const std::string& name() const ;
			std::size_t column_count() { return 0;}
			column_backend* get_column(int i);
			column_backend* get_column(const std::string& name);
			void clear() {}
		};
		const empty_table empty_table_();

		struct empty_db: public db_backend
		{
			const std::string& name()const ;
			const std::string& catalog_name()const ;

			std::size_t table_count() { return 0;}
			table_backend* get_table(int i);
			void get_tables(const std::string& name_pattern) {}
			void clear() {}
		};

		struct empty_meta
		{
			static std::string empty_str_;
			static empty_db empty_db_;
			static empty_column empty_column_;
			static empty_table empty_table_;
		};

	}

	class metadata;
	class db;

	typedef column_backend column;

	class table
	{
		friend class db;
		table_backend* tbl_be_;
		table(table_backend* tbl_be):tbl_be_(tbl_be){}
	public:
		table():tbl_be_(&details::empty_meta::empty_table_){}
		~table(){clear();}

		table_type get_table_type() const { return tbl_be_->get_table_type();}
		const std::string& name() const { return tbl_be_->name();}

		std::size_t column_count() { return tbl_be_->column_count();}
		column& get_column(int i) { return *tbl_be_->get_column(i);}
		column& get_column(const std::string& name)
		{ return *tbl_be_->get_column(name);}

		column& operator[](int i) {return *tbl_be_->get_column(i);}

		column& operator[](const std::string& name)
		{ return *tbl_be_->get_column(name);}

		void clear() {tbl_be_->clear();}
	};

	class db
	{
		friend class metadata;
		db_backend* db_;
		db(db_backend* db):db_(db){}
	public:
		db():db_(&details::empty_meta::empty_db_){}
		~db(){clear();}

		const std::string& name()const { return db_->name();}
		const std::string& catalog_name()const { return db_->catalog_name();}

		std::size_t table_count() { return db_->table_count();}
		table get_table(int i) { return db_->get_table(i);}
		void get_tables(const std::string& name_pattern)
		{db_->get_tables(name_pattern);}

		void clear() {db_->clear();}
	};


	class metadata
	{
		metadata_backend* meta_be_;

	public:
		metadata(metadata_backend* meta_be):meta_be_(meta_be){}

		~metadata(){clear();}

		std::size_t db_count() { return meta_be_->db_count();}

		db get_db(int i) { return meta_be_->get_db(i);}

		void get_dbs(const std::string& name_pattern)
		{ return meta_be_->get_dbs(name_pattern);}

		void clear() {meta_be_->clear();}
	};
}

#endif
