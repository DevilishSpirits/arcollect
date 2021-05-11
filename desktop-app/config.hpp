#pragma once
#include <iostream>
#include <arcollect-paths.hpp>
namespace Arcollect {
	namespace config {
		void read_config(void);
		void write_config(void);
		
		template<typename T>
		class Param {
			private:
				friend void read_config(void);
				T value;
			public:
				/** Read the param
				 */
				inline operator const T&(void) const {
					return value;
				}
				/** Write the param
				 */
				inline Param& operator=(const T& new_value) {
					value = new_value;
					write_config();
					return *this;
				}
				const T default_value;
				Param(const T default_value) : value(default_value), default_value(default_value) {};
		};
	}
}
