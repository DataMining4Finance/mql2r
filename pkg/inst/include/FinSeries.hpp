/*
 * FinSeries.hpp
 *
 *  Created on: Dec 22, 2011
 *      Author: James
 */

#ifndef FINSERIES_HPP_
#define FINSERIES_HPP_

#include <vector>

#include <Rcpp.h>

#include <boost/bimap.hpp>
#include <boost/foreach.hpp>

#include "logger.h"

namespace FinSeries
{

template<typename T>
void applyVisitor(const Rcpp::RObject& object,const T& visitor)
{
	switch(object.sexp_type())
	{
	case NILSXP:
	case SYMSXP:
	case LISTSXP:
	case CLOSXP:
	case ENVSXP:
	case PROMSXP:
	case LANGSXP:
	case SPECIALSXP:
	case BUILTINSXP:
	case CHARSXP:
		//visitor(Rcpp::Vector<CHARSXP>(object));
		break;
	case LGLSXP:
	case INTSXP:
		//visitor(Rcpp::Vector<INTSXP>(object));
		break;
	case REALSXP:
		//visitor(Rcpp::Vector<REALSXP>(object));
		break;
	case CPLXSXP:
	case STRSXP:
		//visitor(Rcpp::Vector<STRSXP>(object));
		break;
	case DOTSXP:
	case ANYSXP:
	case VECSXP:
		visitor(Rcpp::Vector<VECSXP>(object));
		break;
	case EXPRSXP:
	case BCODESXP:
	case EXTPTRSXP:
	case WEAKREFSXP:
	case RAWSXP:
	case S4SXP:
		break;
	default:
		throw std::range_error("Unknown SEXP type");
		break;
	}

}

struct printVisitor
{
	printVisitor(std::string name="",int indent=0) : m_name(name),m_indent(indent)
	{

	}

	template<int I>
	void operator()(const typename Rcpp::traits::storage_type<I>::rtype& val) const
	{
		logger::instance() << prefix() << sexp_to_name(I) << " | " << val << std::endl;
	}

	template<int I>
	void operator()(const Rcpp::Vector<I>& val) const
	{
		for(int i=0;i<val.length();++i)
	    {
	    	applyVisitor(val[i],printVisitor(m_name,m_indent+1));
	    }
	}

	template<unsigned int I>
	void operator()(const Rcpp::Vector<I>& val) const
	{
		//this->operator ()< Rcpp::Vector<(int)I> >(val);
	}

	const std::string prefix() const
	{
		std::ostringstream strm;
		strm << std::string(m_indent,m_sep) << m_name << " | ";
		return strm.str();
	}

	std::string m_name;
	int m_indent;
	const static char m_sep = '\t';
};



void printAttributes(const std::string& name, const Rcpp::RObject& object, int indentation = 0)
{
	std::string indent(indentation, '\t');

    applyVisitor(object,printVisitor(name,indentation));

    logger::instance() << indent << "Attributes of "<< name << ":" << std::endl;

    BOOST_FOREACH(const std::string& attribute, object.attributeNames())
    {
    	logger::instance() << indent << attribute << std::endl;

    	printAttributes(attribute,object.attr(attribute),indentation+1);
    }


    logger::instance() << std::endl;
}

class Xts
{
public:

	   typedef boost::bimap<int,std::string> ColumnMapType;

	Xts(	const std::vector<double>& 			opens,
			const std::vector<double>&			highs,
			const std::vector<double>& 			lows,
			const std::vector<double>&			closes,
			const std::vector<double>& 			volumes,
			const std::vector<Rcpp::Datetime>& 	times) : m_values(opens.size(),5),m_index(opens.size())
	{
		LOGCALL();

		m_columns.insert(ColumnMapType::value_type(0,"Open"));
		m_columns.insert(ColumnMapType::value_type(1,"High"));
		m_columns.insert(ColumnMapType::value_type(2,"Low"));
		m_columns.insert(ColumnMapType::value_type(3,"Close"));
		m_columns.insert(ColumnMapType::value_type(4,"Volume"));

		int len = opens.size();

		for(int i=0;i<len;++i)
		{
			m_values(i,0) = opens[i];
			m_values(i,1) = highs[i];
			m_values(i,2) = lows[i];
			m_values(i,3) = closes[i];
			m_values(i,4) = volumes[i];
			m_index(i) = times[i].getFractionalTimestamp();
		}

	}

    // Constructor from R object.
	Xts(SEXP xtssexp)
	{
			LOGCALL();

			printAttributes("xtssexp",xtssexp);

			//logger::instance() << "Assigning XTS" << std::endl;

			m_values = xtssexp;

			//logger::instance() << "Checking is xts" << std::endl;

		    if(!m_values.inherits("xts"))
		    	throw std::range_error("Invalid class in Xts constructor");

		    //logger::instance() << "getting dimnames" << std::endl;

		    Rcpp::List dims = m_values.attr("dimnames");

		    //Ugly assumptions about dims layout made here.
		    if(dims.length() > 0)
		    {
		    	Rcpp::CharacterVector dimNames = dims[1];

				for(int i=0;i<dimNames.length();++i)
				{
					m_columns.insert(ColumnMapType::value_type(i,std::string(dimNames[i])));
				}
		    }

		    //logger::instance() << "getting index" << std::endl;

		    // Check for date index.
		    m_index = m_values.attr("index");

			print();


	}

	// Create R Object
	operator SEXP() {

		LOGCALL();

		Rcpp::RObject xts;

		xts = m_values;

		Rcpp::CharacterVector xtscv(2);
		xtscv[0] = "xts";
		xtscv[1] = "zoo";
		xts.attr("class") = xtscv;

		Rcpp::List dims(2);

		Rcpp::CharacterVector dimnames(m_columns.size());

		for(size_t i=0;i<m_columns.size();++i)
		{
			dimnames[i] = m_columns.left.at(i);
		}

		dims[1] = dimnames;

		xts.attr("dimnames") = dims;

		xts.attr("index") = m_index;

		Rcpp::RObject indexObj = m_index;

	    Rcpp::CharacterVector indexcv(2);
	    indexcv[0] = "POSIXct";
	    indexcv[1] = "POSIXt";
	    indexObj.attr("class") = indexcv;

		print();

		return xts;
	}

	void print()
	{
		LOGCALL();

	    logger::instance() << "Index contains " << m_index.length() << " entries" << std::endl;

		BOOST_FOREACH(ColumnMapType::left_const_reference i, m_columns.left)
		{
			std::cout 	 << i.second << ",";
		}

		std::cout << std::endl;

		for(int i = 0; i < m_values.rows(); i++)
		{

			if(i < m_index.length())
			{
				const Rcpp::Datetime& time = m_index[i];

				std::cout 	 << "[" << time.getYear() << "-" << time.getMonth() << "-" << time.getDay()
							 << "  " << time.getHours()<< ":" <<time.getMinutes() << ":" << time.getSeconds() << "]  ";
			}
			for(int j = 0; j < m_values.cols(); j++)
			{
				std::cout << m_values(i,j) << ",";
			}
			std::cout << std::endl;
	    }

		std::cout << std::endl;
	}

	size_t nrows() { return m_values.rows(); }
	size_t ncols() { return m_values.cols(); }

	double operator() (int i,int j)
	{
		return m_values(i,j);
	}

	double operator() (int i, const ColumnMapType::right_key_type& col)
	{
		return m_values(i,m_columns.right.at(col));
	}

	double index(int i)
	{
		return m_index[i];
	}
private:


	   ColumnMapType m_columns;
	   Rcpp::NumericMatrix m_values;
	   Rcpp::NumericVector m_index;

};

}

namespace Rcpp {

template<> SEXP wrap<FinSeries::Xts>(const FinSeries::Xts& x) {
	FinSeries::Xts *p = const_cast<FinSeries::Xts*>(&x);
    return (SEXP)*p;
}

}

#endif /* FINSERIES_HPP_ */
