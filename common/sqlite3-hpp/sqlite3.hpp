#pragma once
#include <sqlite3.h>
#include <optional>
#include <string>
#include <string_view>
#include <memory>

namespace SQLite3 {
	// Error code
	constexpr const int OK                = SQLITE_OK;
	// Database opening
	constexpr const int OPEN_READONLY     = SQLITE_OPEN_READONLY;
	constexpr const int OPEN_READWRITE    = SQLITE_OPEN_READWRITE;
	constexpr const int OPEN_CREATE       = SQLITE_OPEN_CREATE;
	constexpr const int OPEN_URI          = SQLITE_OPEN_URI;
	constexpr const int OPEN_MEMORY       = SQLITE_OPEN_MEMORY;
	constexpr const int OPEN_NOMUTEX      = SQLITE_OPEN_NOMUTEX;
	constexpr const int OPEN_SHAREDCACHE  = SQLITE_OPEN_SHAREDCACHE;
	constexpr const int OPEN_PRIVATECACHE = SQLITE_OPEN_PRIVATECACHE;
	constexpr const int OPEN_NOFOLLOW     = SQLITE_OPEN_NOFOLLOW;
	// Datatype
	constexpr const int INTEGER = SQLITE_INTEGER;
	constexpr const int FLOAT   = SQLITE_FLOAT;
	constexpr const int BLOB    = SQLITE_BLOB;
	//constexpr const int NULL    = SQLITE_NULL;
	constexpr const int TEXT    = SQLITE_TEXT;

	struct value {
		/* TODO
		SQLITE_API const void *sqlite3_value_blob(sqlite3_value*);
		SQLITE_API double sqlite3_value_double(sqlite3_value*);
		SQLITE_API int sqlite3_value_int(sqlite3_value*);
		SQLITE_API sqlite3_int64 sqlite3_value_int64(sqlite3_value*);
		SQLITE_API void *sqlite3_value_pointer(sqlite3_value*, const char*);
		SQLITE_API const unsigned char *sqlite3_value_text(sqlite3_value*);
		SQLITE_API const void *sqlite3_value_text16(sqlite3_value*);
		SQLITE_API const void *sqlite3_value_text16le(sqlite3_value*);
		SQLITE_API const void *sqlite3_value_text16be(sqlite3_value*);
		SQLITE_API int sqlite3_value_bytes(sqlite3_value*);
		SQLITE_API int sqlite3_value_bytes16(sqlite3_value*);
		SQLITE_API int sqlite3_value_type(sqlite3_value*);
		SQLITE_API int sqlite3_value_numeric_type(sqlite3_value*);
		SQLITE_API int sqlite3_value_nochange(sqlite3_value*);
		SQLITE_API int sqlite3_value_frombind(sqlite3_value*);
		SQLITE_API unsigned int sqlite3_value_subtype(sqlite3_value*);
		SQLITE_API sqlite3_value *sqlite3_value_dup(const sqlite3_value*);*/
		~value(void) noexcept {
			sqlite3_value_free(reinterpret_cast<sqlite3_value*>(this));
		}
	};

	struct context {
		void* get_auxdata(int N) {
			return sqlite3_get_auxdata(reinterpret_cast<sqlite3_context*>(this),N);
		}
		template <typename T>
		void set_auxdata(int N, T* data, void (*destructor)(T*) = NULL) {
			return sqlite3_set_auxdata(reinterpret_cast<sqlite3_context*>(this),N,static_cast<void*>(data),static_cast<void(*)(void*)>(destructor));
		}
		void *aggregate_context(int nBytes) {
			return sqlite3_aggregate_context(reinterpret_cast<sqlite3_context*>(this),nBytes);
		}
		void *user_data(void) {
			return sqlite3_user_data(reinterpret_cast<sqlite3_context*>(this));
		}
		template <typename T>
		void result(const T* data, int length, void(*destructor)(T*)) {
			return sqlite3_result_blob(reinterpret_cast<sqlite3_context*>(this),static_cast<void*>(data),length,static_cast<void(*)(void*)>(destructor));
		}
		template <typename T>
		void result(const T* data, sqlite3_uint64 length, void(*destructor)(T*)) {
			return sqlite3_result_blob64(reinterpret_cast<sqlite3_context*>(this),static_cast<void*>(data),length,static_cast<void(*)(void*)>(destructor));
		}
		void result(double value) noexcept {
			return sqlite3_result_double(reinterpret_cast<sqlite3_context*>(this),value);
		}
		/*void sqlite3_result_error(sqlite3_context*, const char*, int);
		void sqlite3_result_error16(sqlite3_context*, const void*, int);
		void sqlite3_result_error_toobig(sqlite3_context*);
		void sqlite3_result_error_nomem(sqlite3_context*);
		void sqlite3_result_error_code(sqlite3_context*, int);*/
		void result(int value) noexcept {
			return sqlite3_result_int(reinterpret_cast<sqlite3_context*>(this),value);
		}
		void result(sqlite3_int64 value) noexcept {
			return sqlite3_result_int(reinterpret_cast<sqlite3_context*>(this),value);
		}
		void result_null(void) noexcept {
			return sqlite3_result_null(reinterpret_cast<sqlite3_context*>(this));
		}
		/*void sqlite3_result_text(sqlite3_context*, const char*, int, void(*)(void*));
		void sqlite3_result_text64(sqlite3_context*, const char*,sqlite3_uint64,
				                       void(*)(void*), unsigned char encoding);*/
		/*void sqlite3_result_text16(sqlite3_context*, const void*, int, void(*)(void*));
		void sqlite3_result_text16le(sqlite3_context*, const void*, int,void(*)(void*));
		void sqlite3_result_text16be(sqlite3_context*, const void*, int,void(*)(void*));*/
		/*void sqlite3_result_value(sqlite3_context*, sqlite3_value*);
		void sqlite3_result_pointer(sqlite3_context*, void*,const char*,void(*)(void*));
		void sqlite3_result_zeroblob(sqlite3_context*, int n);
		int sqlite3_result_zeroblob64(sqlite3_context*, sqlite3_uint64 n);*/
		template <typename T>
		context &operator=(const T& value) noexcept {
			result(value);
			return *this;
		}
	};
	
	struct stmt {
		inline int bind(int column, double value) {
			return sqlite3_bind_double((sqlite3_stmt*)this,column,value);
		}
		inline int bind(int column, int value) {
			return sqlite3_bind_int((sqlite3_stmt*)this,column,value);
		}
		inline int bind(int column, sqlite3_int64 value) {
			return sqlite3_bind_int64((sqlite3_stmt*)this,column,value);
		}
		inline int bind(int column, const char* text, int size = -1, void(*dtor)(void*) = NULL) {
			return sqlite3_bind_text((sqlite3_stmt*)this,column,text,size,dtor);
		}
		inline int bind(int column, const std::string_view& text, void(*dtor)(void*) = NULL) {
			return bind(column,text.data(),text.size(),dtor);
		}
		inline int bind_null(int column) {
			return sqlite3_bind_null((sqlite3_stmt*)this,column);
		}
		/* TODO
		int sqlite3_bind_text(sqlite3_stmt*,int,const char*,int,void(*)(void*));
		int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int, void(*)(void*));
		int sqlite3_bind_text64(sqlite3_stmt*, int, const char*, sqlite3_uint64,
				                     void(*)(void*), unsigned char encoding);
		int sqlite3_bind_value(sqlite3_stmt*, int, const sqlite3_value*);
		
		int sqlite3_bind_pointer(sqlite3_stmt*, int, void*, const char*,void(*)(void*));
		int sqlite3_bind_zeroblob(sqlite3_stmt*, int, int n);
		int sqlite3_bind_zeroblob64(sqlite3_stmt*, int, sqlite3_uint64);
		*/
		template <typename T>
		inline int bind(int column, const std::optional<T>& value) {
			return value ? bind(column,*value) :  bind_null(column);
		}
		int column_type(int iCol) {
			return sqlite3_column_type((sqlite3_stmt*)this,iCol);
		}
		bool column_null(int iCol) {
			return column_type(iCol) == SQLITE_NULL;
		}
		/* TODO
		const void *sqlite3_column_blob(sqlite3_stmt*, int iCol);
		*/
		inline double column_double(int iCol) {
			return sqlite3_column_double((sqlite3_stmt*)this,iCol);
		}
		inline int column_int(int iCol) {
			return sqlite3_column_int((sqlite3_stmt*)this,iCol);
		}
		inline sqlite3_int64 column_int64(int iCol) {
			return sqlite3_column_int64((sqlite3_stmt*)this,iCol);
		}
		inline std::optional<sqlite3_int64> column_opt_int64(int iCol) {
			return column_null(iCol) ? std::nullopt : std::make_optional(column_int64(iCol));
		}
		inline const char *column_text(int iCol) {
			return reinterpret_cast<const char*>(sqlite3_column_text((sqlite3_stmt*)this,iCol));
		}
		inline std::string column_string(int iCol) {
			return std::string(column_text(iCol));
		}
		int column_count(void) {
			return sqlite3_column_count((sqlite3_stmt*)this);
		}
		/*
		const void *sqlite3_column_text16(sqlite3_stmt*, int iCol);
		sqlite3_value *sqlite3_column_value(sqlite3_stmt*, int iCol);
		int sqlite3_column_bytes(sqlite3_stmt*, int iCol);
		int sqlite3_column_bytes16(sqlite3_stmt*, int iCol);
		*/
		inline int step(void) {
			return sqlite3_step((sqlite3_stmt*)this);
		}
		inline int reset(void) {
			return sqlite3_reset((sqlite3_stmt*)this);
		}
		
		void operator delete(void* ptr) noexcept {
			sqlite3_finalize((sqlite3_stmt*)ptr);
		}
	};
	struct sqlite3 {
		// Note: v1 interface won't be implemented
		inline int prepare(const char *zSql, int nByte, SQLite3::stmt *&ppStmt, const char **pzTail = NULL) { return sqlite3_prepare_v2((::sqlite3*)this,zSql,nByte,(sqlite3_stmt**)&ppStmt,pzTail); }
		inline int prepare(const char *zSql, int nByte, SQLite3::stmt *&ppStmt, const char *&pzTail) { return this->prepare(zSql,nByte,ppStmt,&pzTail); }
		// std::string versions
		inline int prepare(const std::string_view &Sql, SQLite3::stmt *&ppStmt, const char **pzTail = NULL) { return this->prepare(Sql.data(),Sql.length(),ppStmt,pzTail); }
		inline int prepare(const std::string_view &Sql, SQLite3::stmt *&ppStmt, const char *&pzTail) { return this->prepare(Sql,ppStmt,&pzTail); }
		// std::unique_ptr helper
		inline int prepare(const std::string_view &Sql, std::unique_ptr<SQLite3::stmt> &ppStmt, const char **pzTail = NULL) {
			SQLite3::stmt *stmt;
			int code = this->prepare(Sql,stmt,pzTail);
			if (code == SQLite3::OK)
				ppStmt.reset(stmt);
			return code;
		}
		inline int prepare(const char *zSql, int nByte, std::unique_ptr<SQLite3::stmt> &ppStmt, const char *&pzTail) {
			SQLite3::stmt *stmt;
			int code = this->prepare(zSql,nByte,stmt,pzTail);
			if (code == SQLite3::OK)
				ppStmt.reset(stmt);
			return code;
		}
		
		int exec(const char *sql, int (*callback)(void*,int,char**,char**) = NULL, void *callback_data = NULL, char **errmsg = NULL) {
			return sqlite3_exec((::sqlite3*)this,sql,callback,callback_data,errmsg);
		}
		int exec(const std::string_view &sql, int (*callback)(void*,int,char**,char**) = NULL, void *callback_data = NULL, char **errmsg = NULL) {
			return exec(sql.data(),callback,callback_data,errmsg);
		}
		
		int busy_timeout(int ms) {
			return sqlite3_busy_timeout((::sqlite3*)this,ms);
		}
		
		const char *errmsg(void) {
			return sqlite3_errmsg((::sqlite3*)this);
		}
		
		int extended_errcode(void) {
			return sqlite3_extended_errcode((::sqlite3*)this);
		}
		
		void operator delete(void* ptr) noexcept {
			sqlite3_close_v2((::sqlite3*)ptr);
		}
	};
	
	
	inline int open(const char *filename, sqlite3 *&ppDb) { return sqlite3_open(filename,(::sqlite3**)&ppDb); }
	inline int open(const char *filename, sqlite3 *&ppDb, int flags, const char *zVfs = NULL) { return sqlite3_open_v2(filename,(::sqlite3**)&ppDb,flags,zVfs); }
	
	inline int open(const char *filename, std::unique_ptr<sqlite3> &ppDb) {
		SQLite3::sqlite3 *db;
		int code = SQLite3::open(filename, db);
		if (code == SQLite3::OK)
			ppDb.reset(db);
		return code;
	}
	inline int open(const char *filename, std::unique_ptr<sqlite3> &ppDb, int flags, const char *zVfs = NULL) {
		SQLite3::sqlite3 *db;
		int code = SQLite3::open(filename,db,flags,zVfs);
		if (code == SQLite3::OK)
			ppDb.reset(db);
		return code;
	}
	
	inline int initialize(void) {
		return sqlite3_initialize();
	}
}
