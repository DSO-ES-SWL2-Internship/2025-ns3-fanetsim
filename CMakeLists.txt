build_lib(
  LIBNAME fanet
  SOURCE_FILES
    helper/FANETAddressHelper.cc
    helper/FANETAnimationHelper.cc
    helper/FANETDeviceHelper.cc
    helper/FANETMobilityHelper.cc
    helper/FANETRoutingHelper.cc
    helper/FANETTopologyHelper.cc
  HEADER_FILES
    helper/FANETAddressHelper.h
    helper/FANETAnimationHelper.h
    helper/FANETDeviceHelper.h
    helper/FANETMobilityHelper.h
    helper/FANETRoutingHelper.h
    helper/FANETTopologyHelper.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libstats}
                    ${libcore}
                    ${libwifi}
                    ${libnetwork}
  TEST_SOURCES
)
