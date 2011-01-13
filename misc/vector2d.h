/*---------------------------------------------------------------------

This is a small utility template for 2 dimensional vectors

----------------------------------------------------------------------*/
#pragma once
#include <vector>

//
//
//
template <class T>
class vector2d
{
public:
	//
	// default constructor
	//
	vector2d():m_dimRow(0), m_dimCol(0)
	{
	}

	//
	// constructor
	//
	vector2d(int nRow, int nCol)
	{
		m_dimRow = nRow;
		m_dimCol = nCol;
		for (int i=0; i < nRow; i++)
		{
			std::vector<T> x(nCol);
			int y = x.size();
			m_2DVector.push_back(x);
		}
   }

	//
	//
	//
	void setAt(int nRow, int nCol, const T& value)
	{
		if(nRow >= m_dimRow || nCol >= m_dimCol)
			throw std::out_of_range("Array out of bound");
		else
			m_2DVector[nRow][nCol] = value;
	}
	
	//
	//
	//
	T getAt(int nRow, int nCol)
	{
		if(nRow >= m_dimRow || nCol >= m_dimCol)
			throw std::out_of_range("Array out of bound");
		else
			return m_2DVector[nRow][nCol];
	}

	//
	//
	//
	void growRow(int newSize)
	{
		if (newSize <= m_dimRow)
			return;
		m_dimRow = newSize;
		for(int i = 0 ; i < newSize - m_dimCol; i++)
		{
			std::vector<int> x(m_dimRow);
			m_2DVector.push_back(x);
		}
   }

	//
	//
	//
	void growCol(int newSize)
	{
		if(newSize <= m_dimCol)
			return;
		m_dimCol = newSize;
		for (int i=0; i <m_dimRow; i++)
			m_2DVector[i].resize(newSize);
	}

	//
	//
	//
	std::vector<T>& operator[](int x)
	{
		return m_2DVector[x];
	}

private:
	std::vector< std::vector <T> >            m_2DVector; // the data
	unsigned int                                m_dimRow; // number of rows
	unsigned int                                m_dimCol; // number of columns
};