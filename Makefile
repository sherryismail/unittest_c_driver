PROJECT_BASE	?=	/home/vagrant/dev/SW

GTEST_DIR = $(PROJECT_BASE)/googletest/install

GTEST_FLAGS = -I$(GTEST_DIR)/include -L$(GTEST_DIR)/lib -pthread -lgtest -lgmock

C_FILES =  tx_trf3720_driver.c
CPP_FILES = unittests.cpp TRF3720DriverTest.cpp MockRedirects.cpp

INCLUDES += -I$(PROJECT_BASE)/SOC/Public #driver headers
INCLUDES += -I$(PROJECT_BASE)/Host/linux_apps/UnitTests 
INCLUDES += -I$(PROJECT_BASE)/Public	#SFTypes.h
INCLUDES += -I$(PROJECT_BASE)/Infrastructure/Public	#trace.h
INCLUDES += -I$(PROJECT_BASE)/SOC/tx_trf3720_driver

CXXFLAGS += -g -Wall -Wextra -std=c++11 $(INCLUDES) $(GTEST_FLAGS)

TESTS =
                
all: unittests

clean:
	rm -f $(TESTS) *.o unittests

.PHONY: all clean

MockRedirects.o: MockRedirects.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c MockRedirects.cpp
	
tx_trf3720_driver.o: $(PROJECT_BASE)/SOC/tx_trf3720_driver/tx_trf3720_driver.c
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(PROJECT_BASE)/SOC/tx_trf3720_driver/tx_trf3720_driver.c	

TRF3720DriverTest.o : TRF3720DriverTest.cpp
	$(CXX) $(CXXFLAGS) -c TRF3720DriverTest.cpp
	
# This is the main program that calls all the tests e.g. TRF3720Driver

unittests.o: unittests.cpp
	$(CXX) $(CXXFLAGS) -c unittests.cpp

unittests: unittests.o tx_trf3720_driver.o MockRedirects.o TRF3720DriverTest.o
	$(CXX) $^ $(CXXFLAGS) $(GTEST_FLAGS) \
		 -o $@
