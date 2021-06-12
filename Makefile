

# cd boost_1_76_0
# bash bootstrap.sh
# ./b2 --with-system --with-thread --with-date_time --with-regex --with-serialization stage

BOOST_DIR=./boost_1_76_0

NAME=proxy

CXX=g++

SRCS=main.cpp posix_file_logger.cpp mariadb_listener.cpp tcp_proxy.cpp
OBJS=$(SRCS:.cpp=.o)

CXXFLAGS+=-std=c++11 -I$(BOOST_DIR)/

ASIO_A = \
$(BOOST_DIR)/stage/lib/libboost_date_time.a\
$(BOOST_DIR)/stage/lib/libboost_serialization.a\
$(BOOST_DIR)/stage/lib/libboost_thread.a\
$(BOOST_DIR)/stage/lib/libboost_regex.a\
$(BOOST_DIR)/stage/lib/libboost_system.a\
$(BOOST_DIR)/stage/lib/libboost_wserialization.a

all: depend $(NAME)

depend: .depend

.depend: $(SRCS)
	rm -f "$@"
	$(CXX) $(CXXFLAGS) -MM $^ > "$@"

include .depend

$(NAME): $(OBJS)
	$(CXX) $(OBJS) $(ASIO_A) -pthread $(CXXFLAGS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all






