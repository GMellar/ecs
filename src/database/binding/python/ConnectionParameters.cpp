/*
 *
 *  Created on: 23.05.2021
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * Copyright (C) 2017-2021 Geoffrey Mellar <mellar@gamma-kappa.com>
 * Copyright (C) 2022      Geoffrey Mellar <mellar@house-of-plasma.com>
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
#include <boost/python/call.hpp>
#include <boost/python/object.hpp>
#include <ecs/Database.hpp>
#include <memory>

namespace py = boost::python;

template<typename Container>
class GenericContainerSource {
public:
    typedef typename Container::value_type        char_type;
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
		c.def("execute", &StatementVisitor::executeWrapper);
	}

	static bool bindBlobWrapper(ecs::db3::Statement &self, const std::string &data) {
		return self.bind(std::make_unique<boost::iostreams::stream<GenericContainerSource<std::string>>>(data));
	}

	static std::shared_ptr<ecs::db3::Result> executeWrapper(ecs::db3::Statement &self) {
		return std::make_shared<ecs::db3::Result>(self.execute());
	}
};

struct ResultVisitor : py::def_visitor<ResultVisitor> {
	friend class py::def_visitor_access;

	template<class classT>
	void visit(classT &c) const {
		c.def("fetch", &ResultVisitor::fetchWrapper);
		c.def("isValid", &ResultVisitor::isValidWrapper);
		c.def("__bool__", &ResultVisitor::isValidWrapper);
	}

	static std::shared_ptr<ecs::db3::RowResult> fetchWrapper(ecs::db3::Result &self) {
		return std::make_shared<ecs::db3::RowResult>(self.fetch());
	}

	static bool isValidWrapper(ecs::db3::Result &self) {
		return self;
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

struct RowResultVisitor : py::def_visitor<RowResultVisitor> {
	friend class py::def_visitor_access;

	template<class classT>
	void visit(classT &c) const {
		c.def("isValid", &RowResultVisitor::isValidWrapper);
		c.def("__bool__", &RowResultVisitor::isValidWrapper);
	}

	static bool isValidWrapper(ecs::db3::RowResult &self) {
		return self;
	}
};

class MigrationPythonWrapper : public ecs::db3::Migrator::Migration {
public:
	MigrationPythonWrapper(int from, int to, PyObject *obj) : Migration(from, to), obj(obj) {

	}

	virtual ~MigrationPythonWrapper() {

	}

	virtual bool upMigration(ecs::db3::DbConnection* connection) {
		/* Do not manage the object here */
		boost::python::object o(py::handle<>(py::borrowed(obj)));
		/* Convert the connection to a shared pointer here because the object is not
		 * copyable and python has no pointers at all. We can't pass that as value
		 * either because it is not copyable.
		 */
		return o(std::shared_ptr<ecs::db3::DbConnection>(connection, [](ecs::db3::DbConnection *){}));
	}

private:
	PyObject *obj;
};

struct MigratorVisitor : py::def_visitor<MigratorVisitor> {
	friend class py::def_visitor_access;

	template<class classT>
	void visit(classT &c) const {
		c.def("addMigration", &MigratorVisitor::addMigrationWrapper);
	}

	static void addMigrationWrapper(ecs::db3::Migrator &self,
			int from, int to, const py::object &callable) {
		self.addMigration(std::make_unique<MigrationPythonWrapper>(from, to, callable.ptr()));
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

	py::class_<ecs::db3::RowResult, std::shared_ptr<ecs::db3::RowResult>, boost::noncopyable>("Row", py::no_init)
			.def("size", &ecs::db3::RowResult::size)
			.def("at", &ecs::db3::RowResult::at, py::return_value_policy<py::copy_non_const_reference>())
			.def(RowResultVisitor());
	py::implicitly_convertible<ecs::db3::RowResult, bool>();

	py::class_<ecs::db3::TableBase, std::shared_ptr<ecs::db3::TableBase>, boost::noncopyable>("Table", py::no_init)
			.def("size", &ecs::db3::TableBase::size)
			.def("at", &ecs::db3::TableBase::at, py::return_value_policy<py::copy_non_const_reference>());

	py::class_<ecs::db3::TableResult, boost::noncopyable>("TableResult", py::no_init)
			.def("at", &ecs::db3::TableResult::at, py::return_value_policy<py::copy_non_const_reference>());

	py::class_<ecs::db3::Result, std::shared_ptr<ecs::db3::Result>, boost::noncopyable>("Result", py::no_init)
			.def("getErrorMessage", &ecs::db3::Result::getErrorMessage)
			.def("isValid", &ecs::db3::Result::isValid)
			.def("fetchAll", &ecs::db3::Result::fetchAll)
			.def(ResultVisitor());
	py::implicitly_convertible<ecs::db3::Result, bool>();

	py::class_<ecs::db3::Statement, std::shared_ptr<ecs::db3::Statement>, boost::noncopyable>("Statement", py::no_init)
			.def<bool(ecs::db3::Statement::*)(std::int64_t)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(std::uint64_t)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(const std::string &)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(double)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(float)>("bind", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(float)>("bindFloat", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(double)>("bindDouble", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(std::int64_t)>("bindInt", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(std::uint64_t)>("bindUint", &ecs::db3::Statement::bind)
			.def<bool(ecs::db3::Statement::*)(const std::string &)>("bindString", &ecs::db3::Statement::bind)
			.def(StatementVisitor());

	py::class_<ecs::db3::DbConnection, std::shared_ptr<ecs::db3::DbConnection>, boost::noncopyable>("Connection", py::no_init)
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
			.def("setUseTLS", &ecs::db3::ConnectionParameters::setUseTLS)
			.def("getUseTLS", &ecs::db3::ConnectionParameters::getUseTLS)
			.def("connect", &ecs::db3::ConnectionParameters::connect);

	py::register_ptr_to_python< std::shared_ptr<std::basic_streambuf<char> > >();
	py::register_ptr_to_python< std::shared_ptr<std::basic_istream<char> > >();

	py::class_<ecs::db3::Migrator, std::shared_ptr<ecs::db3::Migrator>, boost::noncopyable>("Migrator", py::init<const std::shared_ptr<ecs::db3::DbConnection> &>())
			.def("initSchema", &ecs::db3::Migrator::initSchema)
			.def("startMigration", &ecs::db3::Migrator::startMigration)
			.def(MigratorVisitor());
}
