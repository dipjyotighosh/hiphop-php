#if EXT_TYPE == 0
"mysql_connect", T(Variant), S(0), "server", T(String), "null_string", S(0), "username", T(String), "null_string", S(0), "password", T(String), "null_string", S(0), "new_link", T(Boolean), "false", S(0), "client_flags", T(Int32), "0", S(0), "connect_timeout_ms", T(Int32), "-1", S(0), "query_timeout_ms", T(Int32), "-1", S(0), NULL, S(0), 
"mysql_pconnect", T(Variant), S(0), "server", T(String), "null_string", S(0), "username", T(String), "null_string", S(0), "password", T(String), "null_string", S(0), "client_flags", T(Int32), "0", S(0), "connect_timeout_ms", T(Int32), "-1", S(0), "query_timeout_ms", T(Int32), "-1", S(0), NULL, S(0), 
"mysql_set_charset", T(Variant), S(0), "charset", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_ping", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_escape_string", T(String), S(0), "unescaped_string", T(String), NULL, S(0), NULL, S(0), 
"mysql_real_escape_string", T(Variant), S(0), "unescaped_string", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_client_encoding", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_close", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_errno", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_error", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_get_client_info", T(String), S(0), NULL, S(0), 
"mysql_get_host_info", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_get_proto_info", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_get_server_info", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_info", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_insert_id", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_stat", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_thread_id", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_create_db", T(Variant), S(0), "db", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_select_db", T(Variant), S(0), "db", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_drop_db", T(Variant), S(0), "db", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_affected_rows", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_set_timeout", T(Boolean), S(0), "query_timeout_ms", T(Int32), "-1", S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_query", T(Variant), S(0), "query", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_unbuffered_query", T(Variant), S(0), "query", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_db_query", T(Variant), S(0), "database", T(String), NULL, S(0), "query", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_list_dbs", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_list_tables", T(Variant), S(0), "database", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_list_fields", T(Variant), S(0), "database_name", T(String), NULL, S(0), "table_name", T(String), NULL, S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_list_processes", T(Variant), S(0), "link_identifier", T(Variant), "null", S(0), NULL, S(0), 
"mysql_db_name", T(Variant), S(0), "result", T(Variant), NULL, S(0), "row", T(Int32), NULL, S(0), "field", T(Variant), "null_variant", S(0), NULL, S(0), 
"mysql_tablename", T(Variant), S(0), "result", T(Variant), NULL, S(0), "i", T(Int32), NULL, S(0), NULL, S(0), 
"mysql_num_fields", T(Variant), S(0), "result", T(Variant), NULL, S(0), NULL, S(0), 
"mysql_num_rows", T(Variant), S(0), "result", T(Variant), NULL, S(0), NULL, S(0), 
"mysql_free_result", T(Variant), S(0), "result", T(Variant), NULL, S(0), NULL, S(0), 
"mysql_data_seek", T(Boolean), S(0), "result", T(Variant), NULL, S(0), "row", T(Int32), NULL, S(0), NULL, S(0), 
"mysql_fetch_row", T(Variant), S(0), "result", T(Variant), NULL, S(0), NULL, S(0), 
"mysql_fetch_assoc", T(Variant), S(0), "result", T(Variant), NULL, S(0), NULL, S(0), 
"mysql_fetch_array", T(Variant), S(0), "result", T(Variant), NULL, S(0), "result_type", T(Int32), "3", S(0), NULL, S(0), 
"mysql_fetch_lengths", T(Variant), S(0), "result", T(Variant), NULL, S(0), NULL, S(0), 
"mysql_fetch_object", T(Variant), S(0), "result", T(Variant), NULL, S(0), "class_name", T(String), "\"stdClass\"", S(0), "params", T(Array), "null", S(0), NULL, S(0), 
"mysql_result", T(Variant), S(0), "result", T(Variant), NULL, S(0), "row", T(Int32), NULL, S(0), "field", T(Variant), "null_variant", S(0), NULL, S(0), 
"mysql_fetch_field", T(Variant), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "-1", S(0), NULL, S(0), 
"mysql_field_seek", T(Boolean), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "0", S(0), NULL, S(0), 
"mysql_field_name", T(Variant), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "0", S(0), NULL, S(0), 
"mysql_field_table", T(Variant), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "0", S(0), NULL, S(0), 
"mysql_field_len", T(Variant), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "0", S(0), NULL, S(0), 
"mysql_field_type", T(Variant), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "0", S(0), NULL, S(0), 
"mysql_field_flags", T(Variant), S(0), "result", T(Variant), NULL, S(0), "field", T(Int32), "0", S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif