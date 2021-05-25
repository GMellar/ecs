/*
 *
 *  Created on: 23.05.2021
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

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/python.hpp>
#include <boost/python/converter/shared_ptr_to_python.hpp>
#include <boost/python/def_visitor.hpp>
#include <ecs/Database.hpp>
#include <memory>

namespace py = boost::python;

template <typename T,
          typename C,
          typename ...Args>
boost::python::object adapt_unique(std::unique_ptr<T> (C::*fn)(Args...))
{
  return boost::python::make_function(
      [fn](C& self, Args... args) { return (self.*fn)(args...).release(); },
      boost::python::return_value_policy<boost::python::manage_new_object>(),
      boost::mpl::vector<T*, C&, Args...>()
    );
}

template<typename Container>
class GenericContainerSource {
public:
    typedef typename Container::value_type  char_type;
    typedef boost::iostreams::seekable_device_tag category;

    GenericContainerSource(const Container& container)
        : container_(container), pos_(0)
        { }

	std::streamsize read(char_type *s, std::streamsize n) {
		using namespace std;
		streamsize amt = static_cast<streamsize>(container_.size() - pos_);
		streamsize result = (min)(n, amt);
		if (result != 0) {
			std::copy(container_.begin() + pos_,
					container_.begin() + pos_ + result, s);
			pos_ += result;
			return result;
		} else {
			return -1; // EOF
		}
	}

	std::streamsize write(const char *s, std::streamsize n) {
		return n;
	}

	std::streampos seek(boost::iostreams::stream_offset off,
			std::ios_base::seekdir way) {
		if(way == std::ios_base::beg) {
			pos_ = 0;
		}else if(way == std::ios_base::cur){
			/* Intentionally left blank */
		}else if(way == std::ios_base::end){
			pos_ = container_.size();
		}

		return pos_;
	}

private:
    typedef typename Container::size_type  size_type;
    Container  container_;
    size_type  pos_;
};

struct StatementVisitor : py::def_visitor<StatementVisitor> {
	friend class py::def_visitor_access;

	template<class classT>
	void visit(classT &c) const {
		c.def("bindBlob", &StatementVisitor::bindBlobWrapper);
	}

	static bool bindBlobWrapper(ecs::db3::Statement &self, const std::string &data) {
		return self.bind(std::make_unique<boost::iostreams::stream<GenericContainerSource<std::string>>>(data));
	}
};

struct CellVisitor : py::def_visitor<CellVisitor> {
	friend class py::def_visitor_access;

	template<class classT>
	void visit(classT &c) const {
		c.def("getBlob", &CellVisitor::getBlob);
	}

	static void getBlob(ecs::db3::types::cell_T &self) {

	}
};

BOOST_PYTHON_MODULE(ecspy) {
	py::class_<ecs::db3::types::cell_T>("Cell", py::no_init)
			.def("getInt", &ecs::db3::types::cell_T::cast_reference<std::int64_t>, py::return_value_policy<py::copy_non_const_reference>())
			.def("getUint", &ecs::db3::types::cell_T::cast_reference<std::uint64_t>, py::return_value_policy<py::copy_non_const_reference>())
			.def("getDouble", &ecs::db3::types::cell_T::cast_reference<double>, py::return_value_policy<py::copy_non_const_reference>())
			.def("getFloat", &ecs::db3::types::cell_T::cast_reference<float>, py::return_value_policy<py::copy_non_const_reference>())
			.def("getString", &ecs::db3::types::cell_T::cast_reference<std::string>, py::return_value_policy<py::copy_non_const_reference>())
			.def("getBlob", &ecs::db3::types::cell_T::cast_reference<std::shared_ptr<std::basic_streambuf<char>>>,
					py::return_value_policy<py::copy_non_const_reference>());

	py::class_<ecs::db3::Row, boost::noncopyable>("Row", py::no_init)
			.def("size", &ecs::db3::Row::size)
			.def("at", &ecs::db3::Row::at, py::return_value_policy<py::copy_non_const_reference>());

	py::class_<ecs::db3::Table, boost::noncopyable>("Table", py::no_init)
			.def("size", &ecs::db3::Table::size)
			.def("at", &ecs::db3::Table::at, py::return_value_policy<py::copy_non_const_reference>());

	py::class_<ecs::db3::Result, boost::noncopyable>("Result", py::no_init)
			.def("getErrorMessage", &ecs::db3::Result::getErrorMessage)
			.def("isValid", &ecs::db3::Result::isValid)
			.def("fetch", adapt_unique(&ecs::db3::Result::fetch))
			.def("fetchAll", adapt_unique(&ecs::db3::Result::fetchAll));

	py::class_<ecs::db3::Statement, boost::noncopyable>("Statement", py::no_init)
			.def("execute", adapt_unique(&ecs::db3::Statement::executePtr))
			.def<bool(ecs::db3::Statement::*)(std::int64_t)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(std::uint64_t)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(const std::string &)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(double)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(float)>("bind", &ecs::db3::Statement::bind)
			.def(StatementVisitor());

	py::class_<ecs::db3::DbConnection, boost::noncopyable>("Connection", py::no_init)
			.def("prepare", &ecs::db3::DbConnection::prepare);


	py::class_<ecs::db3::ConnectionParameters>("ConnectionParameters")
			.def("getDbName", &ecs::db3::ConnectionParameters::getDbName, py::return_value_policy<py::copy_const_reference>())
			.def("setDbName", &ecs::db3::ConnectionParameters::setDbName)
			.def("setDbFilename", &ecs::db3::ConnectionParameters::setDbFilename)
			.def("getDbFilename", &ecs::db3::ConnectionParameters::getDbFilename, py::return_value_policy<py::copy_const_reference>())
			.def("setHostname", &ecs::db3::ConnectionParameters::setHostname)
			.def("getHostname", &ecs::db3::ConnectionParameters::getHostname, py::return_value_policy<py::copy_const_reference>())
			.def("setDbPassword", &ecs::db3::ConnectionParameters::setDbPassword)
			.def("setPort", &ecs::db3::ConnectionParameters::setPort)
			.def("getPort", &ecs::db3::ConnectionParameters::getPort)
			.def("getDbUser", &ecs::db3::ConnectionParameters::getDbUser, py::return_value_policy<py::copy_const_reference>())
			.def("setDbUser", &ecs::db3::ConnectionParameters::setDbUser)
			.def("getBackend", &ecs::db3::ConnectionParameters::getBackend, py::return_value_policy<py::copy_const_reference>())
			.def("setBackend", &ecs::db3::ConnectionParameters::setBackend)
			.def("connect", &ecs::db3::ConnectionParameters::connect);

	py::register_ptr_to_python< std::shared_ptr<ecs::db3::DbConnection> >();
	py::register_ptr_to_python< std::shared_ptr<ecs::db3::Statement> >();
	py::register_ptr_to_python< std::shared_ptr<ecs::db3::Result> >();
	py::register_ptr_to_python< std::shared_ptr<ecs::db3::Row> >();
	py::register_ptr_to_python< std::shared_ptr<ecs::db3::Table> >();
	py::register_ptr_to_python< std::shared_ptr<std::basic_streambuf<char> > >();
	py::register_ptr_to_python< std::shared_ptr<std::basic_istream<char> > >();
}
