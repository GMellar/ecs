/*
 * AbstractPlugin.cpp
 *
 *  Created on: 09.12.2014
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * Copyright (C) 2017 Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef ABSTRACTPLUGIN_HPP_
#define ABSTRACTPLUGIN_HPP_

#if defined(_WIN32)
	#define PLUGIN_API extern "C" __declspec(dllexport)
#else
	#define PLUGIN_API extern "C"
#endif

#include <string>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/config.hpp>

namespace ecs {
namespace plugin {

/** This class provides functions that must be implemented
 * when writing a plugin. Inherit all loadable plugins from
 * this class.
 * @deprecated use ecs::dynlib::AbstractPlugin and ecs::dynlib::LoadableClass instead
 */
class AbstractPlugin {
public:
	POINTER_DEFINITIONS(AbstractPlugin);

	virtual ~AbstractPlugin(){};

	/** Provides plugin version here.
	 *
	 */
	virtual std::string getPluginVersion() = 0;

	/** Provides a detailed description here.
	 *
	 */
	virtual std::string getPluginDescription() = 0;

	/** Provides an author with format: name <email>
	 *
	 */
	virtual std::string getPluginAuthor() = 0;

	/** Provides the plugin name.
	 *
	 */
	virtual std::string getPluginName() = 0;

	/** This function should provide a unique uuid for a plugin
	 * but not for a plugin version. That means the plugin uuid specifies
	 * a specific type of plugin but makes no difference between versions of
	 * a plugin. A chosen UUID for a plugin should NEVER be changed if used for a
	 * database application.
	 *
	 * Some applications will not need it so you can choose to implement it or not. If
	 * you do not implement it, the UUID will be an empty string.
	 */
	virtual std::string getPluginUUID(){return "";};
};

}
}

#endif /* ABSTRACTPLUGIN_HPP_ */
