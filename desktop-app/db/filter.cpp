#include "filter.hpp"
unsigned int Arcollect::db_filter::version = 0;
std::string Arcollect::db_filter::get_sql(void)
{
	return "(art_rating <= " + std::to_string(config::current_rating) + ")";
}
void Arcollect::db_filter::set_rating(config::Rating rating)
{
	config::current_rating = rating;
	Arcollect::db_filter::version++;
}
