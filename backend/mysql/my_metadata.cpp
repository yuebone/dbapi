//
// Created by bon on 15-5-5.
//

#include <vector>
#include "my_metadata.h"
#include "my_backend.h"

namespace dbapi
{
    namespace my
    {
        enum {OLD_META_V=50000};/*5.0*/

        struct change_db
        {
            static const char* metadata_db;

            const std::string& old_db;
            MYSQL* my;

            change_db(MYSQL* my_, const std::string& old_db_) :
                    my(my_),old_db(old_db_)
            {
                if (mysql_select_db(my, metadata_db))
                    _throw_error(my);
            }
            ~change_db()
            {
                if (mysql_select_db(my, old_db.c_str()))
                    _throw_error(my);
            }
        };
        const char* change_db::metadata_db="INFORMATION_SCHEMA";


        my_metadata::my_metadata(my_connection* conn):
                conn_(conn),
                cmd_()
        {
        }

        void my_metadata::clear()
        {
            dbs_.clear();
        }

        std::size_t my_metadata::db_count()
        {
            return dbs_.size();
        }

        void my_metadata::get_dbs(const std::string& patten)
        {

            if(!cmd_)
                cmd_.reset(conn_->create_command());

            result_set_backend* rs_;

            if(conn_->server_version()<OLD_META_V)
            {
                //SHOW DATABASES [LIKE 'pattern']
                if(patten=="")
                    rs_=cmd_->execute_once("SHOW DATABASES");
                else
                    rs_=cmd_->execute_once
                        ((std::string("SHOW DATABASES LIKE '")+
                        patten+"'").c_str());

                dbs_.resize(rs_->row_count(),my_db(this));
                std::vector<my_db>::iterator it=dbs_.begin();

                while (rs_->next_row())
                {
                    it->name_=rs_->get_char_str(0).ptr;
                    ++it;
                }
            }
            else
            {
                change_db change_to_INFORMATION_SCHEMA(conn_->get_mysql(),
                                                       conn_->get_used_db());

                if(patten=="")
                    rs_=cmd_->execute_once("SELECT * FROM SCHEMATA");
                else
                    rs_=cmd_->execute_once
                        ((std::string("SELECT * FROM SCHEMATA WHERE SCHEMA_NAME LIKE '")+
                          patten+"'").c_str());

                int name_i=rs_->pos("SCHEMA_NAME");
                int ctg_i=rs_->pos("CATALOG_NAME");

                dbs_.resize(rs_->row_count(),my_db(this));
                std::vector<my_db>::iterator it=dbs_.begin();

                while (rs_->next_row())
                {
                    it->name_=rs_->get_char_str(name_i).ptr;
                    it->catalog_=rs_->get_char_str(ctg_i).ptr;
                    ++it;
                }
            }
        }

        db_backend* my_metadata::get_db(int i)
        {
            return &dbs_[i];
        }


        void my_db::clear() {tables_.clear(); }

        my_db::my_db(my_metadata *meta) :meta_(meta){ }

        void my_db::get_tables(const std::string &name_pattern)
        {
            result_set_backend* rs_=0;
            if (meta_->conn_->server_version() < OLD_META_V)
            {
                //SHOW TABLE STATUS [FROM db_name] [LIKE 'pattern']
                if (name_pattern=="")
                   rs_=meta_->cmd_->execute_once
                           (std::string("SHOW TABLE STATUS ")+"FROM "+name_);
                else
                    rs_=meta_->cmd_->execute_once
                            (std::string("SHOW TABLE STATUS ")+"FROM "+name_+
                            " LIKE '"+name_pattern+"'");

                tables_.resize(rs_->row_count(),my_table(this));
                std::vector<my_table>::iterator it=tables_.begin();
                while (rs_->next_row())
                {
                    //FIX ME
                    it->table_type_=view;
                    it->name_=rs_->get_char_str(0).ptr;
                    ++it;
                }
            }
            else
            {
                change_db c(meta_->conn_->get_mysql(),meta_->conn_->get_used_db());

                if (name_pattern=="")
                    rs_=meta_->cmd_->execute_once
                            (std::string("SELECT TABLE_NAME,"
                                     "TABLE_TYPE"
                                     " FROM TABLES WHERE TABLE_SCHEMA ='")+
                                     name_+"'");
                else
                    rs_=meta_->cmd_->execute_once
                            (std::string("SELECT TABLE_NAME,TABLE_TYPE"
                                     " FROM TABLES WHERE TABLE_SCHEMA ='")+
                                     name_+"' AND TABLE_NAME LIKE '"+
                                     name_pattern+"'");

                const std::string base_tbl("BASE TABLE");

                tables_.resize(rs_->row_count(),my_table(this));
                std::vector<my_table>::iterator it=tables_.begin();
                while (rs_->next_row())
                {
                    if(base_tbl==rs_->get_char_str(1).ptr)
                        it->table_type_=base_table;
                    else
                        it->table_type_=view;

                    it->name_=rs_->get_char_str(0).ptr;

                    ++it;
                }
            }
        }


        my_table::my_table(my_db *db) :db_(db){ }

        void my_table::clear()
        {
            columns_.clear();
            got_cols_= false;
        }

        std::size_t my_table::column_count()
        {
            if (!got_cols_)
                _get_cols();
            return columns_.size();
        }

        column_backend* my_table::get_column(int i)
        {
            if (!got_cols_)
                _get_cols();
            return &columns_[i];
        }

        static my_column null_col;
        column_backend* my_table::get_column(const std::string &name)
        {
            if (!got_cols_)
                _get_cols();
            std::map<std::string,my_column*>::iterator it=
                    name_map_.find(name);
            if(it==name_map_.end())
                return &null_col;
            return it->second;
        }

        void my_table::_get_cols()
        {
            command_backend* cmd=db_->meta_->cmd_.get();
            my_connection* conn=db_->meta_->conn_;
            const std::string& db_name=db_->name_;
            result_set_backend* rs=0;

            const std::string str_YES("YES");
            const std::string str_decimal("decimal");

            if (conn->server_version() < OLD_META_V)
            {
                //SHOW [FULL] COLUMNS FROM tbl_name [FROM db_name] [LIKE 'pattern']
                rs=cmd->execute_once(std::string("SHOW FULL COLUMNS FROM ")+
                        name_+" FROM "+db_name);

                columns_.resize(rs->row_count());
                std::vector<my_column>::iterator it=columns_.begin();
                //FIX ME
                int name_i=rs->pos("Field");
                int col_type_i=rs->pos("Type");
                int null_i=rs->pos("Null");
                int pos=0;
                while (rs->next_row())
                {
                    it->pos_=pos++;
                    it->name_=rs->get_char_str(name_i).ptr;
                    it->type_name_=rs->get_char_str(col_type_i).ptr;
                    it->required_=str_YES!=rs->get_char_str(null_i).ptr;
                    name_map_[it->name_]=&(*it);
                    ++it;
                }
            }
            else
            {
                change_db c(db_->meta_->conn_->get_mysql(),
                            db_->meta_->conn_->get_used_db());

                rs=cmd->execute_once(std::string("SELECT "
                        "COLUMN_NAME,ORDINAL_POSITION,"
                        "IS_NULLABLE,COLUMN_TYPE,DATA_TYPE,"
                        "NUMERIC_PRECISION,NUMERIC_SCALE"
                        " FROM COLUMNS WHERE")+
                        " TABLE_SCHEMA ='" + db_name+
                        "' AND TABLE_NAME ='" +
                        name_+ "'");
                columns_.resize(rs->row_count());
                std::vector<my_column>::iterator it=columns_.begin();

                while (rs->next_row())
                {
                    it->name_=rs->get_char_str(0).ptr;
                    it->pos_=atoi(rs->get_char_str(1).ptr)-1;
                    it->required_=str_YES!=rs->get_char_str(2).ptr;
                    //FIX ME handle enum_field_types
                    it->type_name_=rs->get_char_str(3).ptr;
                    const char* data_type=rs->get_char_str(4).ptr;
                    if(str_decimal==data_type) {
                        it->num_pty_.precision = atoi(rs->get_char_str(5).ptr);
                        it->num_pty_.scale=atoi(rs->get_char_str(6).ptr);
                    }
                    name_map_[it->name_]=&(*it);
                    ++it;

                }//while
            }//else
            got_cols_= true;
        }//void my_table::_get_cols()

    }//namespace my
}























