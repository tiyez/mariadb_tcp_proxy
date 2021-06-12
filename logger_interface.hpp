

#ifndef Logger_Interface_Hpp
#define Logger_Interface_Hpp

#include <boost/shared_ptr.hpp>
#include <string>

class logger_interface {
public:

	virtual void	open(const std::string &filename) = 0;
	virtual void	close() = 0;
	virtual void	log(std::string) = 0;
	virtual ~logger_interface() {}
};

class logger_factory_interface {
public:

	virtual boost::shared_ptr<logger_interface>	new_logger() = 0;
	virtual ~logger_factory_interface() {}
};


#endif /* Logger_Interface_Hpp */
