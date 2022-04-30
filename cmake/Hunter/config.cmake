if(DEPS_REQUIRED_BOOST_VERSION)
    message(VERBOSE "Setup Boost version requirements: ${DEPS_REQUIRED_BOOST_VERSION}")
	hunter_config(Boost     VERSION ${DEPS_REQUIRED_BOOST_VERSION})
endif()

if(DEPS_REQUIRED_RAPIDJSON_VERSION)
    message(VERBOSE "Setup RapidJSON version requirements: ${DEPS_REQUIRED_RAPIDJSON_VERSION}")
	hunter_config(RapidJSON VERSION ${DEPS_REQUIRED_RAPIDJSON_VERSION})
endif()
