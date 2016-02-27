//
// Created by bon on 15-5-5.
//

#ifndef DBAPI_MY_METADATA_H
#define DBAPI_MY_METADATA_H

#include <vector>
#include <map>
#include "my_def.h"
#include "backend/backend.h"
#include "db_metadata.h"
#include "backend/metadata_backend.h"

namespace dbapi
{
    namespace my
    {

        class my_metadata;
        class my_db;
        class my_table;

        class my_column:public column_backend
        {
            friend class my_table;

            struct numeric_property
            {
                int precision;
                int scale;
            };
        public:

            virtual ~my_column() { }

            virtual const std::string& name() const { return name_;}

            virtual const std::string& type_name() const { return type_name_;}

            virtual int native_type_code() const { return -1;}

            virtual int pos() const { return pos_;}

            virtual bool required() const { return required_;}

            virtual std::size_t size() const { return 0;}

            virtual int precision() const { return num_pty_.precision;}

            virtual int scale() const { return num_pty_.scale;}

        private:

            std::string name_;
            std::string type_name_;//FIX ME using const std::string&
            enum_field_types field_type_;
            int pos_;
            bool required_;
            std::size_t size_;

            union
            {
                numeric_property num_pty_;
            };

        };

        class my_table:public  table_backend
        {
            friend class my_column;
            friend class my_db;
        public:
            virtual ~my_table() { clear();}

            virtual void clear();

            virtual table_type get_table_type() const { return table_type_;}

            virtual const std::string& name() const { return name_;}

            virtual std::size_t column_count() ;

            virtual column_backend* get_column(int i);

            virtual column_backend* get_column(const std::string &name);

        private:
            my_table(my_db* );
            my_db* db_;

            std::vector<my_column> columns_;
            std::map<std::string,my_column*> name_map_;

            table_type table_type_;
            std::string name_;
            bool got_cols_;

            void _get_cols();
        };

        class my_db:public db_backend
        {
            friend class my_column;
            friend class my_table;
            friend class my_metadata;
        public:
            virtual ~my_db() { clear();}

            virtual void clear();

            virtual const std::string& name() const
            { return name_;}

            virtual const std::string& catalog_name() const
            { return catalog_;}

            virtual std::size_t table_count()
            { return tables_.size();}

            virtual table_backend* get_table(int i)
            { return &tables_[i];}

            virtual void get_tables(const std::string &name_pattern) ;



            my_db(my_metadata* meta);

            std::string name_;
            std::string catalog_;

            std::vector<my_table> tables_;

            my_metadata* meta_;
        };

        class my_connection;

        class my_metadata:public metadata_backend
        {
            friend class my_connection;
            friend class my_column;
            friend class my_table;
            friend class my_db;
        public:
            virtual ~my_metadata() { clear(); }

            virtual void clear();

            virtual std::size_t db_count() ;

            virtual db_backend* get_db(int i) ;

            virtual void get_dbs(const std::string &name_pattern) ;


            my_metadata(my_connection* conn);

            my_connection* conn_;

            typedef y::unique_ptr<command_backend> cmd_ptr;
            cmd_ptr cmd_;

            std::vector<my_db> dbs_;
        };
    }
}

#endif //DBAPI_MY_METADATA_H
