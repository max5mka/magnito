// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"

#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>

#define EXPORTED_METHOD extern "C" __declspec(dllexport)

using namespace std;

struct Point2D
{
    double x;
    double z;

    Point2D(double _x = 0, double _z = 0) {
        x = _x;
        z = _z;
    }
};


struct Profile
{
	Point2D coord;
	Point2D B;
};

struct Elem
{
	Point2D begin, end;
	Point2D center;
	Point2D p;
	Point2D gamma;
	double mes;
	double y;
	Elem(double _mes = 0, double _y = 0) {
		mes = _mes;
		y = _y;
	}
};

double I;
double alpha;
double gamma;
int K, n;
int t = 0;

EXPORTED_METHOD
void Initialize1(double I_, int K_, int n_)
{
	I = I_;
	K = K_;
	n = n_;
}

void Initialize2(double alpha_, double gamma_)
{
	alpha = alpha_;
	gamma = gamma_;
}

Point2D operator+ (Point2D p1, Point2D p2) {
	Point2D res;
	res.x = p1.x + p2.x;
	res.z = p1.z + p2.z;
	return res;
}

Point2D operator- (Point2D p1, Point2D p2) {
	Point2D res;
	res.x = p1.x - p2.x;
	res.z = p1.z - p2.z;
	return res;
}

Point2D operator/ (Point2D p1, double number) {
	Point2D res;
	res.x = p1.x / number;
	res.z = p1.z / number;
	return res;
}

Point2D operator* (Point2D p1, double number) {
	Point2D res;
	res.x = p1.x * number;
	res.z = p1.z * number;
	return res;
}

Point2D GetCenter(Elem elem) {
	return (elem.end + elem.begin) / 2.0;
}

double GetMes(Elem elem) {
	return (elem.begin.x - elem.end.x) * (elem.begin.z - elem.end.z) * elem.y;
}

double NormP(double px, double pz) {
	return sqrt(px * px + pz * pz);
}

EXPORTED_METHOD
Elem GetElem(double* X, double* Z, double y, int i, int j) {
	Elem elem;
	elem.begin.x = X[j + 1]; elem.end.x = X[j];
	elem.begin.z = Z[i + 1]; elem.end.z = Z[i];
	elem.center = GetCenter(elem);
	elem.y = 2000.;
	elem.mes = GetMes(elem);
	elem.gamma.x = gamma;
	elem.gamma.z = gamma;
	return elem;
}

void ResizeMatrix(vector<vector<double>>& matrix, size_t row, size_t col) {
	matrix.resize(row);
	for (size_t i = 0; i < row; i++)
		matrix[i].resize(col);
}

//EXPORTED_METHOD
//Elem* GetElems(double* X, int sizeX, double* Z, int sizeZ, double y)
//{
//	size_t n_z = sizeZ - 1;
//	size_t n_x = sizeX - 1;
//	Elem* elements = new Elem[n_x * n_z];
//
//	int c = 0;
//	for (size_t i = 0; i < n_z; i++)
//	{
//		for (size_t j = 0; j < n_x; j++)
//		{
//			elements[c++] = GetElem(X, Z, i, j, y);
//		}
//	}
//	return elements;
//}

EXPORTED_METHOD
double* GetSplit(double start, double end, int N) {
	double* grid = new double[N + 1];
	double step = (end - start) / N;

	int c = 0;
	grid[c++] = start;
	if (N != 1) {
		do {
			start += step;
			grid[c++] = start;
		} while ((end - step) - start > 1e-7);
	}
	grid[c] = end;

	return grid;
}

void MatrixL(vector<vector<double>>& L, Elem* elems, Profile* profile) {

	ResizeMatrix(L, 2 * n, 2 * K);

	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < K; j++)
		{
			Point2D temp = elems[j].center - profile[i].coord;
			double r = sqrt(temp.x * temp.x + temp.z * temp.z);
			double x_z = (temp.x * temp.z) / (r * r);
			double x_x = (temp.x * temp.x) / (r * r);
			double z_z = (temp.z * temp.z) / (r * r);
			double a = elems[j].mes * I / (4 * M_PI * r * r * r);
			L[i * 2][j * 2] = a * (3 * x_x - 1);
			L[i * 2 + 1][j * 2] = a * (3 * x_z);
			L[i * 2][j * 2 + 1] = a * (3 * x_z);
			L[i * 2 + 1][j * 2 + 1] = a * (3 * z_z - 1);
		}
}

vector<vector<double>> Transposition(vector<vector<double>>& L) {
	vector<vector<double>> LT;
	ResizeMatrix(LT, 2 * K, 2 * n);
	for (size_t i = 0; i < 2 * K; i++)
		for (size_t j = 0; j < 2 * n; j++)
			LT[i][j] = L[j][i];
	return LT;
}

vector<vector<int>> NeighboringCells(Elem* elems, int elemsSize) {
	vector<vector<int>> neighbors;
	neighbors.resize(elemsSize);
	for (int i = 0; i < elemsSize; i++)
		for (int j = 0; j < elemsSize; j++) {
			if (((elems[j].end.x >= elems[i].end.x) && (elems[j].end.x < elems[i].begin.x))
				&& ((elems[j].end.z == elems[i].begin.z) || (elems[j].begin.z == elems[i].end.z)))
				neighbors[i].push_back(j);

			if ((elems[j].end.z == elems[i].end.z) && (elems[j].begin.z == elems[i].begin.z)
				&& ((elems[j].end.x == elems[i].begin.x) || (elems[i].end.x == elems[j].begin.x)))
				neighbors[i].push_back(j);
		}
	return neighbors;
}

void AlphaRegularization(vector<vector<double>>& A) {
	for (size_t i = 0; i < 2 * K; ++i)
		A[i][i] += alpha;
}

vector<vector<double>> MatrixC(Elem* elems, vector<vector<int>> neighbors) {
	vector<vector<double>> C;
	ResizeMatrix(C, K * 2, K * 2);

	for (size_t i = 0; i < neighbors.size(); i++)
		for (size_t j = 0; j < neighbors[i].size(); j++)
		{
			int k = neighbors[i][j];
			C[i * 2][k * 2] = -(elems[i].gamma.x + elems[j].gamma.x);
			C[i * 2 + 1][k * 2] = -(elems[i].gamma.z + elems[j].gamma.x);
			C[i * 2][k * 2 + 1] = -(elems[i].gamma.x + elems[j].gamma.z);
			C[i * 2 + 1][k * 2 + 1] = -(elems[i].gamma.z + elems[j].gamma.z);
		}

	for (size_t i = 0; i < neighbors.size(); i++) {
		size_t size = neighbors[i].size();
		Point2D sum;
		for (size_t j = 0; j < size; j++) {
			sum.x += elems[neighbors[i][j]].gamma.x;
			sum.z += elems[neighbors[i][j]].gamma.z;
		}
		C[i * 2][i * 2] = size * elems[i].gamma.x + sum.x;
		C[i * 2 + 1][i * 2 + 1] = size * elems[i].gamma.z + sum.z;
	}
	return C;
}

void IncreaseGammas(Elem*& elems, int elemsSize, vector<vector<int>>& neighbors) {
	vector<int> isChangedx, isChangedz;
	double coef = 5.0;

	for (int i = 0; i < elemsSize; i++)
	{
		for (int j = 0; j < neighbors[i].size(); j++)
		{
			int k = neighbors[i][j];
			if ((max(elems[i].p.x, elems[k].p.x) / min(elems[i].p.x, elems[k].p.x)) <= coef)
			{
				if (find(isChangedx.begin(), isChangedx.end(), i) == isChangedx.end()) {
					isChangedx.push_back(i);
					elems[i].gamma.x *= coef;
				}

				if (find(isChangedx.begin(), isChangedx.end(), k) == isChangedx.end()) {
					isChangedx.push_back(k);
					elems[k].gamma.x *= coef;
				}
			}

			if ((max(elems[i].p.z, elems[k].p.z) / min(elems[i].p.z, elems[k].p.z)) <= coef)
			{
				if (find(isChangedz.begin(), isChangedz.end(), i) == isChangedz.end()) {
					isChangedz.push_back(i);
					elems[i].gamma.z *= coef;
				}

				if (find(isChangedz.begin(), isChangedz.end(), k) == isChangedz.end()) {
					isChangedz.push_back(k);
					elems[k].gamma.z *= coef;
				}
			}
		}
	}
}

EXPORTED_METHOD
Point2D GetB(Point2D coordProfile, Elem elem) {
	Point2D B;
	Point2D tmp = elem.center - coordProfile;
	double r = sqrt(tmp.x * tmp.x + tmp.z * tmp.z);
	double x_z = (tmp.x * tmp.z) / (r * r);
	double x_x = (tmp.x * tmp.x) / (r * r);
	double z_z = (tmp.z * tmp.z) / (r * r);
	B.x = (elem.p.x * (3 * x_x - 1) + elem.p.z * (3 * x_z));
	B.z = (elem.p.x * (3 * x_z) + elem.p.z * (3 * z_z - 1));
	B = B * elem.mes * I / (4 * M_PI * r * r * r);
	return B;
}

EXPORTED_METHOD
void CalcB(Profile* profile, int profileSize, Elem* object, int objectSize) 
{
	for (size_t i = 0; i < profileSize; i++) 
	{
		Point2D res;
		for (size_t k = 0; k < objectSize; k++)
		{
			res = res + GetB(profile[i].coord, object[k]);
			//cout << "k = " << k << ", res = (" << res.x << ", " << res.z << ")";
			t++;
		}
			
		profile[i].B = res;
	}
}

double GetFunc(Profile* profile, int profileSize, Elem* inverse_elems, int objectSize) {
	vector<Point2D> real_B;
	double func = 0.;

	for (size_t i = 0; i < n; i++) {
		Point2D temp;
		temp.x = profile[i].B.x;
		temp.z = profile[i].B.z;
		real_B.push_back(temp);
	}

	for (size_t i = 0; i < profileSize; i++)
	{
		Point2D res;
		for (size_t k = 0; k < objectSize; k++)
			res = res + GetB(profile[i].coord, inverse_elems[k]);
		profile[i].B = res;
	}

	for (size_t i = 0; i < n; i++)
		func += (real_B[i].x - profile[i].B.x) * (real_B[i].x - profile[i].B.x) + (real_B[i].z - profile[i].B.z) * (real_B[i].z - profile[i].B.z);

	return func;
}

void Gauss(vector<vector<double>>& A, vector<double>& b, Elem*& elems) {
	vector<double>p;
	p.resize(2 * K, 0);

	for (size_t i = 0; i < p.size(); i++)
		p[i] = b[i];
	for (int k = 0; k < 2 * K - 1; k++)
		for (int i = k + 1; i < 2 * K; i++)
		{
			double elem = -A[i][k] / A[k][k];
			for (int j = k; j < 2 * K; j++)
				A[i][j] += elem * A[k][j];
			p[i] += elem * p[k];
		}

	for (int i = 2 * K - 1; i >= 0; i--)
	{
		double elem = 0;
		for (int j = i + 1; j < 2 * K; j++)
			elem += A[i][j] * p[j];
		p[i] = (p[i] - elem) / A[i][i];
	}

	for (int k = 0; k < K; k++) {
		elems[k].p.x = p[k * 2];
		elems[k].p.z = p[k * 2 + 1];
	}
}

void GammaRegularization(vector<vector<double>> A, vector<double> b, 
	Elem*& elems, int elemsSize, Profile* profile, int profileSize)
{
	vector<vector<int>> neighbors = NeighboringCells(elems, elemsSize);
	double coef = 10, Fstart, F = 0.0;
	vector<vector<double>> C = MatrixC(elems, neighbors);
	vector<vector<double>> fullA;
	int maxiter = 50, k = 0;

	ResizeMatrix(fullA, 2 * K, 2 * K);

	for (int i = 0; i < 2 * K; i++)
		for (int j = 0; j < 2 * K; j++)
			fullA[i][j] = A[i][j] + C[i][j];

	Gauss(fullA, b, elems);
	Fstart = GetFunc(profile, profileSize, elems, elemsSize);
	cout << Fstart << "\n";
	do {
		IncreaseGammas(elems, elemsSize, neighbors);
		C = MatrixC(elems, neighbors);
		for (int i = 0; i < 2 * K; i++)
			for (int j = 0; j < 2 * K; j++)
				fullA[i][j] = A[i][j] + C[i][j];

		Gauss(fullA, b, elems);
		F = GetFunc(profile, profileSize, elems, elemsSize);
		cout << F << endl;
		k++;
	} while (F / Fstart < coef && k < maxiter);
}

void MatrixA(vector<vector<double>>& A, vector<vector<double>>& L) {
	ResizeMatrix(A, 2 * K, 2 * K);
	vector<vector<double>> LT = Transposition(L);
	for (size_t i = 0; i < 2 * K; ++i)
		for (size_t j = 0; j < 2 * K; ++j) {
			A[i][j] = 0;
			for (int k = 0; k < 2 * n; ++k) {
				A[i][j] += LT[i][k] * L[k][j];
			}
		}
}

void MatrixB(vector<double>& b, vector<vector<double>>& L, Profile* profiles) {
	vector<vector<double>> LT = Transposition(L);
	vector<double>S;
	S.resize(2 * n, 0);
	b.resize(2 * K, 0);

	for (size_t i = 0; i < n; i++)
	{
		S[i * 2] = profiles[i].B.x;
		S[i * 2 + 1] = profiles[i].B.z;
	}

	for (size_t i = 0; i < 2 * K; ++i)
		for (size_t j = 0; j < 2 * n; ++j)
			b[i] += LT[i][j] * S[j];
}

EXPORTED_METHOD
void CalcP(double* normP, Profile* profile, int profileSize, Elem* inverse_elems, int elemsSize) {
	vector<vector<double>> L;
	vector<vector<double>> A;
	vector<double> b;

	//MatrixL
	L.resize(2 * n);
	for (size_t i = 0; i < 2 * n; i++)
		L[i].resize(2 * K);

	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < K; j++)
		{
			Point2D temp = inverse_elems[j].center - profile[i].coord;
			double r = sqrt(temp.x * temp.x + temp.z * temp.z);
			double x_z = (temp.x * temp.z) / (r * r);
			double x_x = (temp.x * temp.x) / (r * r);
			double z_z = (temp.z * temp.z) / (r * r);
			double a = inverse_elems[j].mes * I / (4 * M_PI * r * r * r);
			L[i * 2][j * 2] = a * (3 * x_x - 1);
			L[i * 2 + 1][j * 2] = a * (3 * x_z);
			L[i * 2][j * 2 + 1] = a * (3 * x_z);
			L[i * 2 + 1][j * 2 + 1] = a * (3 * z_z - 1);
		}




	// MatrixA
	A.resize(2 * K);
	for (size_t i = 0; i < 2 * K; i++)
		A[i].resize(2 * K);

	vector<vector<double>> LT = Transposition(L);
	for (size_t i = 0; i < 2 * K; ++i)
		for (size_t j = 0; j < 2 * K; ++j) {
			A[i][j] = 0;
			for (int k = 0; k < 2 * n; ++k) {
				A[i][j] += LT[i][k] * L[k][j];
			}
		}




	// MatrixB
	vector<vector<double>> LT1 = Transposition(L);
	vector<double>S;
	S.resize(2 * n, 0);
	b.resize(2 * K, 0);

	for (size_t i = 0; i < n; i++)
	{
		S[i * 2] = profile[i].B.x;
		S[i * 2 + 1] = profile[i].B.z;
	}

	for (size_t i = 0; i < 2 * K; ++i)
		for (size_t j = 0; j < 2 * n; ++j)
			b[i] += LT1[i][j] * S[j];




	// AlphaRegularization
	for (size_t i = 0; i < 2 * K; ++i)
		A[i][i] += alpha;





	// GAMMAREGULARIZATION
	vector<vector<int>> neighbors = NeighboringCells(inverse_elems, elemsSize);
	double coef = 10, Fstart = 0., F = 0.0;
	vector<vector<double>> C = MatrixC(inverse_elems, neighbors);
	vector<vector<double>> fullA;
	int maxiter = 50, k = 0;
	
	//Resize
	fullA.resize(2 * K);
	for (size_t i = 0; i < 2 * K; i++)
		fullA[i].resize(2 * K);

	for (int i = 0; i < 2 * K; i++)
		for (int j = 0; j < 2 * K; j++)
			fullA[i][j] = A[i][j] + C[i][j];

	//Gauss
	vector<double>p;
	p.resize(2 * K, 0);

	for (size_t i = 0; i < p.size(); i++)
		p[i] = b[i];
	for (int k = 0; k < 2 * K - 1; k++)
		for (int i = k + 1; i < 2 * K; i++)
		{
			double elem = -fullA[i][k] / fullA[k][k];
			for (int j = k; j < 2 * K; j++)
				fullA[i][j] += elem * fullA[k][j];
			p[i] += elem * p[k];
		}

	for (int i = 2 * K - 1; i >= 0; i--)
	{
		double elem = 0;
		for (int j = i + 1; j < 2 * K; j++)
			elem += fullA[i][j] * p[j];
		p[i] = (p[i] - elem) / fullA[i][i];
	}

	for (int k = 0; k < K; k++) {
		inverse_elems[k].p.x = p[k * 2];
		inverse_elems[k].p.z = p[k * 2 + 1];
	}

	//GetFunc
	vector<Point2D> real_B;

	for (size_t i = 0; i < n; i++) {
		Point2D temp;
		temp.x = profile[i].B.x;
		temp.z = profile[i].B.z;
		real_B.push_back(temp);
	}

	for (size_t i = 0; i < profileSize; i++)
	{
		Point2D res;
		for (size_t k = 0; k < elemsSize; k++)
			res = res + GetB(profile[i].coord, inverse_elems[k]);
		profile[i].B = res;
	}

	for (size_t i = 0; i < n; i++)
		Fstart += (real_B[i].x - profile[i].B.x) * (real_B[i].x - profile[i].B.x) + (real_B[i].z - profile[i].B.z) * (real_B[i].z - profile[i].B.z);


	cout << Fstart << "\n";
	do {
		IncreaseGammas(inverse_elems, elemsSize, neighbors);
		C = MatrixC(inverse_elems, neighbors);
		for (int i = 0; i < 2 * K; i++)
			for (int j = 0; j < 2 * K; j++)
				fullA[i][j] = A[i][j] + C[i][j];

		Gauss(fullA, b, inverse_elems);
		F = GetFunc(profile, profileSize, inverse_elems, elemsSize);
		cout << F << endl;
		k++;
	} while (F / Fstart < coef && k < maxiter);





	// GAUSS
	/*vector<double>p1;
	p1.resize(2 * K, 0);

	for (size_t i = 0; i < p1.size(); i++)
		p1[i] = b[i];
	for (int k = 0; k < 2 * K - 1; k++)
		for (int i = k + 1; i < 2 * K; i++)
		{
			double elem = -A[i][k] / A[k][k];
			for (int j = k; j < 2 * K; j++)
				A[i][j] += elem * A[k][j];
			p1[i] += elem * p1[k];
		}

	for (int i = 2 * K - 1; i >= 0; i--)
	{
		double elem = 0;
		for (int j = i + 1; j < 2 * K; j++)
			elem += A[i][j] * p1[j];
		p1[i] = (p1[i] - elem) / A[i][i];
	}

	for (size_t k = 0; k < K; k++) {
		inverse_elems[k].p.x = p1[k * 2];
		inverse_elems[k].p.z = p1[k * 2 + 1];
	}*/




	// GETFUNC
	/*vector<Point2D> real_B1;
	Func = 0.;

	for (size_t i = 0; i < n; i++) {
		Point2D temp;
		temp.x = profile[i].B.x;
		temp.z = profile[i].B.z;
		real_B1.push_back(temp);
	}

	for (size_t i = 0; i < profileSize; i++)
	{
		Point2D res;
		for (size_t k = 0; k < elemsSize; k++)
			res = res + GetB(profile[i].coord, inverse_elems[k]);
		profile[i].B = res;
	}

	for (size_t i = 0; i < n; i++)
		Func += (real_B1[i].x - profile[i].B.x) * (real_B1[i].x - profile[i].B.x) + (real_B1[i].z - profile[i].B.z) * (real_B1[i].z - profile[i].B.z);*/





	for (size_t k = 0; k < K; k++) {
		normP[k] = NormP(inverse_elems[k].p.x, inverse_elems[k].p.z);
	}
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

