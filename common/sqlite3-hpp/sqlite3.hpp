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
