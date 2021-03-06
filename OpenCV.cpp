#pragma once
#include "stdafx.h"

using namespace cv;
using namespace std;

const ushort mask16 = 0xFF00;	// folosit la stergerea celui mai NEsemnificativ octet dintr-un cuvant
const uchar masks[6] = { 0, 254, 252, 248, 240, 224 };		// stergerea celor mai NEsemnificativi biti dintr-un octet
const uchar getLast[8] = { 0, 1, 3, 7, 15, 31, 63, 127 };	// obtinerea celor mai NEsemnificativi biti dintr-un octet
const ushort getFirst[13] = { 0, 0x80, 0xC0, 0xE0, 0x8000, 0xC000, 0xE000 };
								// obtinerea celor mai SEMNIFICATIVI biti dintr-un octet/cuvant

string toBin(size_t val) {
	return bitset<8>(val).to_string();
}
unsigned long toDec(string val) {
	return bitset<8>(val).to_ulong();
}
string toBin16(int val) {
	return bitset<16>(val).to_string();
}
unsigned long toDec16(string val) {
	return bitset<16>(val).to_ulong();
}
string toBin32(size_t val) {
	return bitset<32>(val).to_string();
}
unsigned long toDec32(string val) {
	return bitset<32>(val).to_ulong();
}

string citesteDate(string fileName, char tip = 0) 
{
		string msg, dat;
		ifstream f(fileName, ios::binary & tip);
		if (!f) { return "Fisierul cu acest nume nu exista"; }

		if (tip == 0) {
			while (getline(f, dat)) 
			{
				msg += dat;
				msg += '\n';
			}
			return msg;
		}
		else {
			string content((std::istreambuf_iterator<char>(f)), (std::istreambuf_iterator<char>()));
			return content;
		}	
}

void scriereDate(string fileName, string msg, char tip = 0) 
{
	ofstream f(fileName,ios::binary & tip);
	f << msg;	
}


//----------------------------------------------------------------------------------------------------------
//------------------------------------------ FUNCTII CRIPTARE/ DECRIPTARE ----------------------------------
//----------------------------------------------------------------------------------------------------------


/*template<class T>		//-----------------------------------VERSIUNEA 2 CRIPTARE XOR---------------------
void criptDecriptInfo_2(T &info, string key, uchar &st, uchar len) {
	uchar i;									// functie pentru criptarea rezolutiei	/ initiatorului de sir
	hash<string>pass;							// mai jos este descrisa ideea folosirii [st]
	for (i = 0; i < len; i++) {
		key += st;
		info[i] ^= pass(key);
		key.pop_back();
		st++;
		if (st == 127)st = '0';
	}
}

Mat criptDecriptMat_2(Mat src, string &key, uchar &st) {// [st] = contor global, folosit pentru alterarea imaginii
	ushort i, j;									//  pe scurt: Daca nu se foloseste [st], in imaginea secreta, desi XOR-ata,
	hash<string>pass;								//  culorile sunt schimbate total, dar conturile sunt VIZIBILE. Practic = 0.
	for (i = 0; i < src.rows; i++)					//  [st] adauga la [key] un caracter, de la '0' la ultimul caracter ASCII = 126,
		for (j = 0; j < src.cols * 3; j++) {		//  apoi il sterge, astfel pentru fiecare octet din [src] / [info],
			key += st;								//  va fi folosita alta cheie de XOR.
			src.at<uchar>(i, j) ^= pass(key);		//  La extragere este folosita aceeasi functie, [st] pornind tot de la 0,
			key.pop_back();							//  se respecta ordinea XOR-arii pentru fiecare caracter
			st++;
			if (st == 127) {
				st = '0';					// se reseteaza contorul global si se concateneaza caracterul 0, pentru a
			}										// asigura unicitatea fiecarui hash obtinut
		}
	return src;
}*/			// ---------------------------- SFARSIT VERSIUNE 2 -----------------


string chain(string &key, uchar len)
{
	for (uchar i = len - 1; i >= 0; i--)
	{
		key[i]++;
		if (key[i] != 126)
			break;
		else
			key[i] = '!';
	}
	return key;
}

template<class T>
void criptDecriptInfo_3(T &info, string &key, int len)
{
	uchar k = (uchar)key.length() - 1, v1;
	int i;
	size_t val;
	hash<string>pass;

	for (i = 0; i < len; i++)	// cripteaza [info] cu permutarile [key]
	{					
		val = pass(key); 
		for (v1 = 0; v1 < 8; v1++) 
		{
			info[i] ^= val;
			val >>= 8;
		}
		key[k]++;
		if (key[k] == 127) 
		{
			key[k] = '!'; 
			chain(key, k);
		}
	}
}

Mat criptDecriptMat_3(Mat src, string &key)
{
	ushort i, j;
	uchar k = (uchar)key.length() - 1, v1, v2;
	size_t val;
	hash<string>pass;

	for (i = 0; i < src.rows; i++)
		for (j = 0; j < src.cols * 3; j++)
		{
			val = pass(key);
			v2 = src.at<uchar>(i, j);
			for (v1 = 0; v1 < 8; v1++) 
			{
				v2 ^= val;
				val >>= 8;
			}
			src.at<uchar>(i, j) = v2;
			key[k]++;
			if (key[k] == 127) 
			{
				key[k] = '!'; 
				chain(key, k);
			}

		}
	//cout << endl << "FINAL:" << key << " " << key.length() << endl;
	return src;
}

string criptare(string msg, string key)		// criptarea folosita la ascunderea / extragerea textului
{			
	int len = (int)msg.length(), i;
	uchar ch;
	string binCry, repBin;
	hash<string> pass;

	for (i = 0; i < len; i++) 
	{
		ch = (uchar)(msg[i] ^ pass(key));
		binCry = toBin(ch);
		repBin += binCry;
	}
	return repBin;
}

//----------------------------------------------------------------------------------------------------------
//------------------------------------------ ASCUNDERE / EXTRAGERE TEXT ------------------------------------
//----------------------------------------------------------------------------------------------------------

uchar estePassCorectMSG(Mat img, string key)
{
	hash<string> pass;
	string msg, buf;
	uchar lit;
	int bt, j = 0;
	for (bt = 1; bt < 5 ;bt++) 
	{
		while (msg.length() != 3) 
		{
			buf += toBin(img.at<uchar>(0, j)).substr(8 - bt, bt);
			if (buf.length() >= 8) 
			{
				lit = (uchar)toDec(buf.substr(0, 8));
				lit ^= pass(key);
				msg += lit;
				buf.erase(0, 8);
			}
			j++;
		}
		if (msg == "%^&")return bt;
		msg = ""; buf = ""; j = 0;
	}
	return 0;
}
Mat LSBAscundere(Mat img, string msg, string key, uchar &bt) 
{
	int len, btCont = 0;
	string repBin, valCanal;
	ushort i, j;

	// adaugare identificator inceput si sfarsit de mesaj
	msg = "%^&" + msg + "@#$";

	bt = 1;
	len = (int)msg.length();

	// se decide nr de biti nesemnificativi potrivit pe care sa se scrie mesajul, [bt]
	while ((img.rows*img.cols * 3) / (8.0 / bt) < len * (8.0 / bt)) 
		bt++;
	

	// de la 4 biti in sus imaginea se deformeaza prea mult
	if (bt >= 5) 
		 return Mat::zeros(1, 1, CV_8U);
	
	
	// criptez mesajul cu ajutorul cheii primite de la user si transform in binar
	repBin = criptare(msg, key);

	// adaug 2 caractere deoarece cand [bt] = 3, e posibil ca ultima
	// portiune din [repBin] sa nu fie scrisa corect in imagine
	if (bt == 3) 
		repBin += "10";

	len *= 8;
	int stPos = 8 - bt;

	for (i = 0; i < img.rows && btCont <= len; i++) 
	{
		for (j = 0; j < img.cols * 3 && btCont <= len; j++) 
		{
			//  iau valoarea unuia dintre canale si o convertesc in binar 
			valCanal = toBin(img.at<uchar>(i, j));

			//  iau primii [bt] biti din reprezentarea binara 
			valCanal.replace(stPos, bt, repBin.substr(btCont, bt));
			
			//  a mesajului de intrare si ii plasez peste ultimii [bt]  biti ai [valCanal]
			//  transform [valCanal] in zecimal si rescriu inapoi de unde l-am luat
			img.at<uchar>(i, j) = (uchar)toDec(valCanal);
			btCont += bt;
		}
	}

	/*// daca [bt] e impar, ultima portiune din ultimul byte nu va fi scrisa corect
	if (valCanal.length() < 8) 
	{
		while (valCanal.length() != 8) 
		{
			// corectez aceasta variabila adaugandu-i 0-uri
			valCanal += '0';
		}
		img.at<uchar>(i - 1, j - 1) = (uchar)toDec(valCanal);
	}*/
	return img;
}

string LSBExtragere(Mat img, string key, uchar &bt) 
{
	bt = estePassCorectMSG(img, key);
	if (!bt) return "$#Pwdincorect#$";

	uchar lit,  virLen = 0, stPos = 8 - bt;
	string msg, buffer;
	hash<string> pass;
	ushort i, j;
	int in8 = 0, lenMsg = 0;

	for (i = 0; i < img.rows; i++)
		for (j = 0; j < img.cols * 3; j++) 
		{
			// iau valoarea canalului si o transform in binar = [valCanal]
			// adaug cei mai nesemnificativi [bt] biti ai [valCanal] in [buffer]
			buffer += toBin(img.at<uchar>(i, j)).substr(stPos, bt);

			// [virLen] este lungimea bufferului, din [buffer] voi lua cate 8 biti, pentru a forma o litera
			virLen += bt;
			if (virLen > 8) 
			{
				// memorez ultimele litere aparute(in cautarea terminatorului de sir)
				// obtin reprezentarea binara a 8 biti din buffer si transform in zecimal
				lit = (uchar)toDec(buffer.substr(in8, 8));

				// decriptez litera la care am ajuns
				// si o adaug in mesajul final
				lit ^= pass(key);
				msg += lit;
				lenMsg++;

				// daca am dat peste terminator de sir, afisez mesajul
				// tai inceputul si sfarsitul sirului, [msg] = mesajul extras
				if (lenMsg > 6) 
				{
					if (msg.substr(lenMsg - 3, 3) == "@#$") 
					{
						msg = msg.substr(3, lenMsg - 6); goto RET;
					}
				}
				virLen -= 8;

				// [in8] e folosit pentru a face sarituri din 8 in 8 biti
				in8 += 8;
			}
		}
	RET: return msg;
}



//------------------------------------------------------------------------------------------------------
//---------------------------- ASCUNDERE / EXTRAGERE IMAGINE SAU EXECUTABIL 16 BITI --------------------
//------------------------------------------------------------------------------------------------------

ushort setLast8Bits(uchar src, ushort dest) 
{
	dest &= mask16;				// [mask16] este constanta = 65280, folosita pentru a sterge ultimii 8 biti ai [dest]
	dest |= src;				// folosesc OR logic pe biti pentru a copia sursa peste destinatie
	return dest;
}

template<class T>
uchar getLast8Bits(T src) 
{
	return src &= 255;			// aplic AND logic pentru a sterge cei mai semnificativi 8 biti
}

int estePassCorectEXE(Mat cov, string &key, string init) 
{
	string binMsg;				// [binMsg] este sirul binar extras din cover
	char val[4], j;
	int nrCaractere = 0;

	for (j = 0; j < init.length(); j++) 
	{
		val[0] = (char)getLast8Bits(cov.at<ushort>(0, j));
		criptDecriptInfo_3(val, key, 1);
		binMsg += val[0];
	}
	if (binMsg != init) return 0;							// verifica identificator

	for (j = 3; j < 7; j++)									// extrage lungimea sirului binar
		val[j - 3] = getLast8Bits(cov.at<ushort>(0, j));

	criptDecriptInfo_3(val, key, 4);
	for (j = 3; j >= 0; j--) 
	{
		nrCaractere <<= 8;
		nrCaractere |= (uchar)val[j];
	}
	return nrCaractere;
}

string extExecutabil(Mat cov, string key) 
{
	string init = "@!#";
	uchar val, initLen = (uchar)init.length();
	string msg;
	int msgLen = estePassCorectEXE(cov, key, init), i;

	if (!msgLen)  return "passIncorect"; 

	ushort x = 1, y = 0;
	for (i = 0; i < msgLen; i++) 
	{
		val = getLast8Bits(cov.at<ushort>(x, y));
		msg += (char)val;
		y++;
		if (y == cov.cols * 3) 
		{
			x++;
			y = 0;
		}
	}
	criptDecriptInfo_3(msg, key, msgLen);
	return msg;
}

Mat ascExecutabil(string msg, Mat cov, string key) 
{ 
	ushort x = 0, y = 0;
	string init = "@!#";
	uchar val, initLen = (uchar)init.length(), j;
	int msgLen = (int)msg.length(), i;

	if (cov.depth() == 0)
		cov.convertTo(cov, CV_16U, 255);

	i = msgLen;
	for (j = 0; j < 4; j++) 
	{
		init += (char)getLast8Bits(msgLen); 
		msgLen >>= 8;
	}
	msgLen = i;
	initLen += 4;
	criptDecriptInfo_3(init, key, initLen);

	for (j = 0; j < initLen; j++)
		cov.at<ushort>(0, j) = setLast8Bits(init[j], cov.at<ushort>(0, j));

	criptDecriptInfo_3(msg, key, msgLen);
	x = 1;
	for (i = 0; i < msgLen; i++) 
	{
		val = msg[i];
		cov.at<ushort>(x, y) = setLast8Bits(val, cov.at<ushort>(x, y));
		y++;
		if (y == cov.cols*3) 
		{
			x++;
			y = 0;
		}
	}
	return cov;
}


// verifica daca initiatorul de sir este valabil pentru cheia introdusa
char estePassCorectIMG(Mat cov, string &key, string init) 
{
	string binMsg;		// [binMsg] este sirul binar extras din cover
	char val[2], j, i = 0;

	for (j = 0; j < init.length(); j++)
	{
		val[0] = (char)cov.at<ushort>(i, j);
		criptDecriptInfo_3(val, key, 1);
		binMsg += val[0];
	}
	if (binMsg == init) return 1;
	return 0;
}

Mat ascImgLossless(Mat msg, Mat cov, string key)   // ---------------------------- ascundere pe 16 biti		
{		
	ushort i, j, rez[2];
	string init = "&^%";		// initializator / identificator imagine
	uchar val, initLen = (uchar)init.length();

	//============= convertesc coverul la 16 biti, 1 byte devine 1 cuvant(2 bytes) ===================
	if (cov.depth() == 0)
		cov.convertTo(cov, CV_16U, 255);

	//  [msg] trebuie neaparat sa fie pe 8 biti
	if (msg.depth() != 0)
		msg.convertTo(msg, CV_8U, 1 / 255.0);

	// criptez rezolutia si initiatorul de sir prin XOR-are cu hash-ul cheii citite de la user.
	rez[0] = msg.rows;					// latime
	rez[1] = msg.cols;					// lungime
	criptDecriptInfo_3(init, key, initLen);
	criptDecriptInfo_3(rez, key, 2);

	// concatenez rezolutia de initiatorul de sir
	j = 0;
	for (i = initLen; i < initLen + 4; i += 2) 
	{
		init[i] = rez[j] >> 8;				// pe octetul [i] adaug primii 8 biti din dimensiune
		init[i + 1] = (uchar)rez[j];		// pe octetul [i+1] adaug urmatorii 8 biti, astfel memorez 1 cuvant pe 4 octeti
		j++;
	}

	//  NOTATIE: 1 N = cei mai nesemnificativi 8 biti din 1 cuvant al cover-ului.
	//  structura: primii 3 N reprezinta initiatorul de sir(pentru verificare password)
	//  urmatorii 4 N reprezinta rezolutia imaginii
	i = 0;
	initLen += 4;
	for (j = 0; j < initLen; j++)
		cov.at<ushort>(i, j) = setLast8Bits(init[j], cov.at<ushort>(i, j));

	// criptez imaginea mesaj
	criptDecriptMat_3(msg, key);

	// modific 1 N, ca sa memorez intensitatile imaginii mesaj
	for (i = 1; i < msg.rows + 1; i++) 
	{
		for (j = 0; j < msg.cols * 3; j++) 
		{
			val = msg.at<uchar>(i - 1, j);
			cov.at<ushort>(i, j) = setLast8Bits(val, cov.at<ushort>(i, j));
		}
	}
	return cov;
}

Mat extImgLossless(Mat cov, string key)			//----------------------------- extragere de pe 16 biti
{				
	ushort i, j = 0, rez[2];
	string init = "&^%";
	uchar val, initLen = (uchar)init.length();

	// verific passwordul
	if (!estePassCorectIMG(cov, key, init)) return Mat::zeros(1, 1, CV_8U);

	// extrag din imagine rezolutia criptata si o XOR-ez cu cheia userului + permutari
	val = 0;
	for (i = initLen; i < initLen + 4; i += 2)  // primele 3 caractere = identificatorul; a fost verificat
	{	
		rez[val] = getLast8Bits(cov.at<ushort>(j, i));
		rez[val] <<= 8;
		rez[val] |= getLast8Bits(cov.at<ushort>(j, i + 1));
		val++;
	}
	criptDecriptInfo_3(rez, key, 2);

	// creez o matrice finala, depth de 8 biti, in care "desenez" imaginea gasita in cover
	Mat fin(rez[0], rez[1], CV_8UC3);

	//  cu ultimii 8 biti din fiecare canal al coverului, decriptez si creez noua imagine
	for (i = 1; i <= fin.rows; i++) 
	{
		for (j = 0; j < fin.cols * 3; j++) 
		{
			val = (uchar)cov.at<ushort>(i, j);
			fin.at<uchar>(i - 1, j) = val;
		}
	}
	criptDecriptMat_3(fin, key);
	return fin;
}


//---------------------------------------------------------------------------------------------------------------
//---------------------------- ASCUNDERE / EXTRAGERE IMAGINE PE 8 BITI(HECHT) -----------------------------------
//---------------------------------------------------------------------------------------------------------------


template<class T>
uchar getFirstNBits(T src, uchar n)		// formula pentru uchar / ushort
{						
	return (src & getFirst[sizeof(T) * sizeof(T) + n - 1]) >> (sizeof(T) * 8 - n);
}

void setLastNBits(uchar src, uchar &dest, uchar n)
{
	dest &= masks[n];
	src &= ~masks[n];
	dest |= src;
}

uchar getLastNBits(ushort src, uchar n) 
{
	return src & getLast[n];
}


void get3BytesMSG(uchar src, uchar *rez)   //(ASCUNDERE)
{			
	rez[0] = (src & 28) >> 2;		// urmatorii 3 -> scrisi in B	
	rez[1] = (src & 96) >> 5;		// urmatorii 2 biti semnificativi  -> scrisi in G
	rez[2] = (src & 128) >> 7;		// iau cel mai semnificativ bit din [src] -> va fi scris in canalul R de catre setBitsToCOV
}

void set3BytesCOV(uchar *src, uchar *dest)   //(ASCUNDERE)
{			
	int i;
	for (i = 0; i < 3; i++) 
	{								// src = getBitsFromMSG( 3 octeti), dest = 1 pixel al coverului(3 octeti)
		dest[i] &= masks[3 - i];	// sterge bitul Cel Mai Nesemnificativ pentru canalul R, c.m.n 2 biti pentru G, 3 pentru B
		dest[i] |= src[i];			// suprapune bitii din sursa peste destinatie
	}
}
														
uchar getHecht(uchar *src) 
{										// primeste un pixel din cover si il converteste intr-un octet al imaginii ascunse:
	uchar rez = 0;						// iau c.m.n bit din canalul R, shiftez [rez] spre stanga 
	rez |= (src[2] & 1);				// ca sa fac loc si celorlaltor componente( G -> 2 biti, B -> 3 biti)
	rez <<= 2;							
	rez |= (src[1] & 3);				
	rez <<= 3;
	rez |= (src[0] & 7);								
	rez <<= 2;
	return rez;
}

void separateWord(ushort src, uchar *rez)  // separa 1 cuvant in 6 octeti care contin(3-3-3-3-2-2) biti fiecare
{	
	int i;
	for (i = 0; i < 4; i++) 
	{
		rez[i] = getFirstNBits(src, 3);
		src <<= 3;
	}
	for (i = 4; i < 6; i++) 
	{
		rez[i] = getFirstNBits(src, 2);
		src <<= 2;
	}
}

void setRezolutie(Mat cov, ushort lung, ushort lat, string &key) 
{												// [lung] = cols, [lat] = rows
	uchar i, j, x = 0;							//  lungimea si latimea -> vor fi scrise in [cov]							
	uchar sepXY[2][6];							//  impartite in C.M.N 3-3-3-3-2-2 biti ai primilor 6 octeti  
	ushort vRez[2] = { lung, lat };				//  respectiv 3-3-3-3-2-2 biti ai urmatorilor 6 octeti 
												//  (12 octeti necesari in pentru rezolutie)

	string init = "#stl";						// initializator de sir pt pass
	criptDecriptInfo_3(init, key, (uchar)init.length());					   // criptez initializ.	

	for (i = 0; i < init.length(); i++) 
	{
		for (j = 0; j < 4; j++) 
		{
			setLastNBits(getFirstNBits(init[i], 2), cov.at<uchar>(0, x++), 2);
			init[i] <<= 2;			// la fel ca mai sus, dupa cei 12 octeti ce reprezinta rezolutia,	
									// voi scrie 16 octeti ,ai caror C.M.N 2 biti formeaza initializatorul de imagine
		}							// TOTAL NECESAR INITIALIZATOR: 38 octeti din cover
	}

	criptDecriptInfo_3(vRez, key, 2);			//  se cripteaza rezolutia
	separateWord(vRez[0], sepXY[0]);			//  lung -> pe prima linie,
	separateWord(vRez[1], sepXY[1]);			//  lat -> pe a 2-a linie din [sepXY]

	for (i = 0; i < 2; i++)      // i = 0 => scrie lungimea, i = 1 => scrie latimea in cover
	{					
		for (j = 0; j < 4; j++) 
		{			
			setLastNBits(sepXY[i][j], cov.at<uchar>(0, x), 3);		
			x++;
		}
		for (j = 4; j < 6; j++) {
			setLastNBits(sepXY[i][j], cov.at<uchar>(0, x), 2);		
			x++;
		}
	}
	
}

uchar getRezolutie(Mat cov, ushort *rez,string &key) 
{											// opusul setRezolutie(), folosit la extragere, rez[0] = cols , rez[1] = rows
	uchar i, j, p, x = 0;						
	rez[0] = rez[1] = 0;							
	string init = "#stl", fin;											

	for (i = 0; i < init.length(); i++)      // obtin initializatorul de sir		
	{					
		p = 0;
		for (j = 0; j < 4; j++) 
		{
			p <<= 2;										// de 4 ori: pun cate 2 biti din cover in octetul final, 
			p |= getLastNBits(cov.at<uchar>(0, x), 2);		// shiftez la stanga cu 2 pozitii ca sa fac loc urmatorilor biti,
			x++;											// trec la octetul urmator
		}
		fin += p;											// concatenez octetul la stringul final.
	}
	criptDecriptInfo_3(fin, key, (uchar)init.length());		// la sfarsit, decriptez initializatorul

	for (p = 0; p < 2; p++)    // extrag rezolutia
	{								
		for (i = 0; i < 4; i++) 
		{
			rez[p] <<= 3;
			rez[p] |= getLastNBits(cov.at<uchar>(0, x), 3);
			x++;
		}
		for (i = 4; i < 6; i++) 
		{
			rez[p] <<= 2;
			rez[p] |= getLastNBits(cov.at<uchar>(0, x), 2);
			x++;
		}
	}
	criptDecriptInfo_3(rez, key, 2);							// si o decriptez

	if (fin == init)return 1;			
	return 0;
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------SFARSIT FUNCTII PENTRU HECHT------------------------------------------
// ---------------------------------------------------------------------------------------------------------

Mat ascImgHecht(Mat msg, Mat cov, string key) 
{
	ushort i, j, x = 0, y = 1;
	uchar obt[3];

	if (msg.rows*msg.cols > (cov.rows*cov.cols) / 3.0) {		// daca imaginea mesaj este mai mare decat cover
		return Mat::zeros(1, 1, CV_8U);
	}
	setRezolutie(cov, msg.cols, msg.rows, key);		// ascund si criptez rezolutia si initiatorul de sir in [cov]
	criptDecriptMat_3(msg, key);					// criptez imaginea secreta

	for (i = 1; i < msg.rows + 1; i++) {					// pe B scriu 3 biti , G -> 2 biti , R -> 1 bit
		for (j = 0; j < msg.cols * 3; j++) {
			get3BytesMSG(msg.at<uchar>(i - 1, j), obt);
			set3BytesCOV(obt, cov.ptr(y, x));
			x++;
			if (x == cov.cols) {
				y++;
				x = 0;
			}
		}
	}
	return cov;
}

Mat extImgHecht(Mat cov, string key) 
{
	ushort i, j, x = 0, y = 0, rez[2];
	uchar val;

	if (!getRezolutie(cov, rez, key)) 				// se verifica passwordul
		return Mat::zeros(1, 1, CV_8U);

	Mat fin(rez[1], rez[0], CV_8UC3);					// creez imaginea finala cu dimensiunile extrase din cover
	for (i = 1; i < cov.rows; i++)
		for (j = 0; j < cov.cols; j++) 
		{
			val = getHecht(cov.ptr(i, j));					// iau C.M.N (3biti pt Blue, 2biti pt Green, 1 pentru Red) 
			cout << val << " " << toBin(val) << endl;
			_getwch();
			fin.at<uchar>(y, x) = val;						// din fiecare pixel, si il compun in [fin]
			x++;
			if (x == rez[0] * 3)
			{												// 4 indecsi sunt necesari(i,j,x,y) pentru ca operez 
				y++;										// pe 2 imagini diferite la pozitii diferite
				x = 0;
				if (y == rez[1]) 
				{
					criptDecriptMat_3(fin, key);		// decriptez imaginea
					return fin;
				}
			}
		}
	return Mat::zeros(2, 2, CV_8U);
}