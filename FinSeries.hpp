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

#include <boost/foreach.hpp>

namespace FinSeries
{
/*
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
		std::cout << prefix() << sexp_to_name(I) << " | " << val << std::endl;
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

    std::cout << indent << "Attributes of "<< name << ":" << std::endl;

    BOOST_FOREACH(const std::string& attribute, object.attributeNames())
    {
    	std::cout << indent << attribute << std::endl;

    	printAttributes(attribute,object.attr(attribute),indentation+1);
    }


    std::cout << std::endl;
}
*/
class Xts
{
public:
	Xts(	const std::vector<double>& 			opens,
			const std::vector<double>&			highs,
			const std::vector<double>& 			lows,
			const std::vector<double>&			closes,
			const std::vector<double>& 			volumes,
			const std::vector<Rcpp::Datetime>& 	times) :
				m_opens(opens),
				m_highs(highs),
				m_lows(lows),
				m_closes(closes),
				m_volumes(volumes),
				m_times(times)
	{


	}

    // Constructor from R object.
	Xts(SEXP xtssexp)
	{
		   Rcpp::RObject xts(xtssexp);

		   printAttributes("xts",xts);

		    if(!xts.inherits("xts"))
		    	throw std::range_error("Invalid class in Xts constructor");

		    Rcpp::List dims = xts.attr("dimnames");

		    Rcpp::CharacterVector dimNames = dims[1];


		    if(! ( dimNames[0] == "Open" && dimNames[1] == "High" && dimNames[2] == "Low" && dimNames[3] == "Close") )
		    {
		    	throw std::range_error("Invalid dimension names in Xts constructor");
		    }


		    // Check for date index.
		    Rcpp::RObject indexAttr = xts.attr("index");
		    Rcpp::RObject indexClass = Rcpp::RObject(indexAttr).attr("class");
		    if(indexClass != R_NilValue) { // applies to date indexes
		    	Rcpp::CharacterVector ind(indexClass);
				if(std::string(ind[0]) != "POSIXct")
				{
					throw std::range_error("Invalid index type of " + ind[0] +" in Xts constructor");
				}
		    }

		    int indexLength = Rf_length(indexAttr);

			Rcpp::NumericVector nv(indexAttr);
			m_times.resize(indexLength);
			for(int i = 0; i < indexLength; ++i)
			{
			    m_times[i] = Rcpp::Datetime(nv(i));
		    }

			Rcpp::NumericMatrix nm(xts);
			int nrows = nm.rows();
			m_opens.resize(nrows);
			m_highs.resize(nrows);
			m_lows.resize(nrows);
			m_closes.resize(nrows);
			m_volumes.resize(nrows);

			for(int i=0; i < nrows; ++i) {
			    m_opens[i] = nm(i,0);
			    m_highs[i] = nm(i,1);
			    m_lows[i] = nm(i,2);
			    m_closes[i] = nm(i,3);
			    m_volumes[i] = nm(i,4);
			}

			print();
	}

	// Create R Object
	operator SEXP() {
		Rcpp::RObject xts;

		int nrows = m_opens.size();
		int ncols = 5;

		Rcpp::NumericMatrix nm(nrows,ncols);
		xts = nm;

		Rcpp::CharacterVector xtscv(2);
		xtscv[0] = "xts";
		xtscv[1] = "zoo";
		xts.attr("class") = xtscv;



		Rcpp::List dims(2);

		Rcpp::CharacterVector dimnames(ncols);

		dimnames[0] = "Open";
		dimnames[1] = "High";
		dimnames[2] = "Low";
		dimnames[3] = "Close";
		dimnames[4] = "Volume";

		dims[1] = dimnames;

		xts.attr("dimnames") = dims;

		Rcpp::NumericVector index(nrows);


		for(int i = 0; i < nrows; i++)
		{
			nm(i,0) = m_opens[i];
			nm(i,1) = m_highs[i];
			nm(i,2) = m_lows[i];
			nm(i,3) = m_closes[i];
			nm(i,4) = m_volumes[i];
			index(i) = m_times[i].getFractionalTimestamp();
	    }

		xts.attr("index") = index;

		Rcpp::RObject indexObj = index;

	    Rcpp::CharacterVector indexcv(2);
	    indexcv[0] = "POSIXct";
	    indexcv[1] = "POSIXt";
	    indexObj.attr("class") = indexcv;

		//print();

		return xts;
	}

	void print()
	{
		int nrows = m_opens.size();


		std::cout 	 << "Time,Open,High,Low,Close,Volume" << std::endl;


		for(int i = 0; i < nrows; i++)
		{
			const Rcpp::Datetime& time = m_times[i];

			std::cout 	 << time.getYear() << "-" << time.getMonth() << "-" << time.getDay() << ":" << time.getHours()<< ":" << time.getSeconds() << ","
						 << m_opens[i] << ","
						 << m_highs[i] << ","
						 << m_lows[i] << ","
						 << m_closes[i] << ","
						 << m_volumes[i] << std::endl;
	    }
	}

	std::vector<double>& Opens()	{ return m_opens; }
	std::vector<double>& Highs() 	{ return m_highs; }
	std::vector<double>& Lows() 	{ return m_lows; }
	std::vector<double>& Closes() 	{ return m_closes; }
	std::vector<double>& Volumes()  { return m_volumes; }
	std::vector<Rcpp::Datetime>& Times() { return m_times; }

private:

	   std::vector<double> 			m_opens;
	   std::vector<double> 			m_highs;
	   std::vector<double> 			m_lows;
	   std::vector<double> 			m_closes;
	   std::vector<double> 			m_volumes;
	   std::vector<Rcpp::Datetime> 	m_times;

};

}

namespace Rcpp {

template<> SEXP wrap<FinSeries::Xts>(const FinSeries::Xts& x) {
	FinSeries::Xts *p = const_cast<FinSeries::Xts*>(&x);
    return (SEXP)*p;
}

}

#endif /* FINSERIES_HPP_ */
