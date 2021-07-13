#pragma once
#include <dbus/dbus.h>
namespace DBus {
	typedef DBusBusType BusType;
	constexpr const DBusBusType BUS_SESSION = DBUS_BUS_SESSION;
	constexpr const DBusBusType BUS_SYSTEM  = DBUS_BUS_SYSTEM;
	constexpr const DBusBusType BUS_STARTER = DBUS_BUS_STARTER;
	
	template<typename T, T*(*ref)(T*), void(*unref)(T*)>
	class DBusRefCounted {
		private:
			T *const internal;
		protected:
			inline DBusRefCounted(T *pointer) : internal(pointer) {};
		public:
			inline DBusRefCounted(DBusRefCounted &other) : internal(ref(other.internal)) {};
			inline ~DBusRefCounted(void) {unref(internal);};
			operator T*(void) {
				return internal;
			}
	};
	
	class Connection: public DBusRefCounted<DBusConnection,dbus_connection_ref,dbus_connection_unref> {
		public:
			Connection(BusType type, DBusError *error = NULL) : DBusRefCounted(dbus_bus_get(type,error)) {}
			Connection(DBusConnection *connection) : DBusRefCounted(connection) {dbus_connection_ref(connection);}
			
			inline int bus_request_name(const char *name, unsigned int flags,DBusError *error = NULL) {
				return dbus_bus_request_name(*this,name,flags,error);
			}
			
			inline void set_exit_on_disconnect(dbus_bool_t exit_on_disconnect) {
				return dbus_connection_set_exit_on_disconnect(*this,exit_on_disconnect);
			}
			inline dbus_bool_t send(DBusMessage *message,dbus_uint32_t *serial = NULL) {
				return dbus_connection_send(*this,message,serial);
			}
			inline dbus_bool_t read_write_dispatch(int timeout_milliseconds = -1) {
				return dbus_connection_read_write_dispatch(*this,timeout_milliseconds);
			}
			inline dbus_bool_t read_write(int timeout_milliseconds = -1) {
				return dbus_connection_read_write(*this,timeout_milliseconds);
			}
			inline dbus_bool_t dispatch(void) {
				return dbus_connection_dispatch(*this);
			}
	};
	
	struct append_iterator: public DBusMessageIter {
		inline dbus_bool_t append_basic(int type, const void *value) {
			return dbus_message_iter_append_basic(this,type,value);
		}
		inline dbus_bool_t append_fixed_array(int element_type, const void *value, int n_elements) {
			return dbus_message_iter_append_fixed_array(this,element_type,value,n_elements);
		}
		inline dbus_bool_t open_container(int type, const char *contained_signature, append_iterator &sub) {
			return dbus_message_iter_open_container(this,type,contained_signature,&sub);
		}
		inline dbus_bool_t close_container(append_iterator &sub) {
			return dbus_message_iter_close_container(this,&sub);
		}
		
		append_iterator& operator<<(const char* string) {
			append_basic('s',&string);
			return *this;
		}
		
		inline append_iterator(void) = default;
		inline append_iterator(const append_iterator&) = default;
		inline append_iterator(DBusMessage* message) {
			dbus_message_iter_init_append(message,this);
		}
	};
	class Message {
		public:
			struct iterator: public DBusMessageIter {
				bool not_past_end = true;
				inline dbus_bool_t has_next(void) {
					return dbus_message_iter_has_next(this);
				}
				inline dbus_bool_t next(void) {
					return dbus_message_iter_next(this);
				}
				inline int get_arg_type(void) {
					return dbus_message_iter_get_arg_type(this);
				}
				inline int get_element_type(void) {
					return dbus_message_iter_get_element_type(this);
				}
				inline iterator recurse(void) {
					iterator iter;
					dbus_message_iter_recurse(this,&iter);
					return iter;
				}
				//char * 	dbus_message_iter_get_signature (DBusMessageIter *iter)
				inline void get_basic(void *value) {
					return dbus_message_iter_get_basic(this,value);
				}
				template <typename T>
				inline T get_basic(void) {
					T value;
					get_basic(&value);
					return value;
				}
				inline int get_element_count(void) {
					return dbus_message_iter_get_element_count(this);
				}
				inline void get_fixed_array(void *value, int &n_elements) {
					return dbus_message_iter_get_fixed_array(this,value,&n_elements);
				}
				
				inline iterator &operator++(void) {
					next();
					not_past_end = has_next();
					return *this;
				}
				inline iterator &operator*(void) {
					return *this;
				}
				// This is hacky but works here...
				inline bool operator !=(iterator& other) {
					return not_past_end != other.not_past_end;
				}
				iterator begin(void) {
					return recurse();
				}
				// Don't mind
				iterator end(void) {
					iterator iter;
					iter.not_past_end = false;
					return iter;
				}
				
				inline iterator(void) = default;
				inline iterator(const iterator&) = default;
				inline iterator(DBusMessage* message) {
					dbus_message_iter_init(message,this);
				}
			};
			/*iterator begin(void) {
				iterator iter;
				dbus_message_iter_init(this,&iter);
				return iter;
			}
			// Don't mind
			iterator &end(void) {
				return *reinterpret_cast<iterator*>(this);
			}*/
	};
}
