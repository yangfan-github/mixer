#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include "../inc/mixerEngine.h"
#include "../inc/media.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using namespace boost;
extern media_thread_pool g_pool;

#endif // STDAFX_H_INCLUDED
