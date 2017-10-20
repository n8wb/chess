#include "board.h"
#include "move.h"
//const char columns[8] = {'a','b','c','d','e','f','g','h'};

using namespace std;

Board::Board(string FEN, string spec, string castle){
	this->tiles = (Tile***)calloc(sizeof(Tile**),8);
	string f = FEN;
	char c;
	cout<<"Processing the Castle data...";
	this->bCastle = ((castle.find("bk")==string::npos)? 0 : KING_SIDE) | ((castle.find("bq")==string::npos)? 0 : QUEEN_SIDE);
	this->wCastle = ((castle.find("wk")==string::npos)? 0 : KING_SIDE) | ((castle.find("wq")==string::npos)? 0 : QUEEN_SIDE);
	cout<<"done, WHITE - "<<out->wCastle<<" : BLACK - "<<out->bCastle<<endl;
	this->taken = -1;
	this->history = nullptr;
	cout<<"Creating the tiles\n";
	for(int i = 0;i<8;i++){
		this->tiles[i] = (Tile**)calloc(sizeof(Tile*),8);
		for(int j = 0;j<8;j++){
			c = getLast(f);
			f = substring(f,0,-1);
			if(c=='/'){
				c = getLast(f);
				f = substring(f,0,-1);
			}
			this->tiles[i][j] = new Tile(i+1,j+1,c);
		}	
	}
	cout<<"Created tiles\n";
	this->specialData(spec);
	cout<<"Processed the special data\n";
	
}

Board::~Board(){
	for(int i = 0;i<8;i++){
		for(int j = 0;j<8;j++){
			delete this->tiles[i][j];
		}
		free(this->tiles[i]);
	}
	free(this->tiles);
}

Tile* Board::getTile(int row, int col){
	return this->tiles[row-1][col-1];
}


void Board::castle(Piece* p,char side){
	int r = p->loc->row;
	int c = p->loc->col;
	if(side==KING_SIDE){
		this->forceChange(r,c+2,p->FEN);
		this->forceChange(r,c,EMPTY_SPACE);
		this->forceChange(r,c+1,this->getTile(r,8)->p->FEN);
		this->forceChange(r,8,EMPTY_SPACE);
		p->move(new Location(r,c+2));
		this->getTile(r,c+1)->p->move(new Location(r,c+1));
		
	}else{
		this->forceChange(b,r,c-2,p->FEN);
		this->forceChange(b,r,c,EMPTY_SPACE);
		this->forceChange(b,r,c-1,this->getTile(r,1)->p->FEN);
		this->forceChange(b,r,1,'X');
		p->move(new Location(r,c-2));
		this->getTile(r,c-1)->p->move(new Location(r,c-1));
	}
	return;
}

void Board::specialData(string data){
	cout<<"Getting the data from the string\n";
	int r = data[0] - 48;
	int c = data[1] - 48;
	cout<<"POS is ("<<r<<","<<c<<") data is "<<data<<endl;
	char side = data[2];
	cout<<"Checking to see if tile is valid\n";
	if(this->getTile(r,c)==nullptr || this->getTile(r,c)->empty()){
		return;
	}
	cout<<"The tile is not empty and not NULL\n";
	if(! this->getTile(r,c)->p->is(PAWN)){
		return;
	}
	cout<<"Setting the special bit\n";
	if(side =='r'){
		this->getTile(r,c)->p->setSpecial(RIGHT);
	}else{
		this->getTile(r,c)->p->setSpecial(LEFT);
	}
}

void Board::forceChange(int r,int c,char FEN){
	delete this->tiles[r-1][c-1];
	this->tiles[r-1][c-1] = new Tile(r,c,FEN);
}

bool Board::placePiece(int r,int c,char FEN){
	if(!this->getTile(r,c)->empty() || Piece::getPieceName(FEN)==0){
		return false;
	}
	this->forceChange(r,c,FEN);
	return true;
}

Location* Board::findKing(char side){
	Tile* t;
	for(int i = 1;i<9;i++)
		for(int j = 1;j<9;j++){
			t = this->getTile(i,j);
			if(!t->empty() && t->p->is(KING) && t->p->side == side){
				return t->p->loc;
			}	
	}
	cerr<<"ERROR: Unable to the find the king\n";
	return nullptr;
}

Piece Board::getKing(char side){
	Location* loc = this->findKing(side);
	return this->getTile(loc->row,loc->col)->p;
}

uint8_t Board::inCheck(char side){
	Location* loc = this->findKing(side);
	Tile* t;
	int i,j;
	for(int i = 1;i<9;i++)
		for(int j = 1;j<9;j++){
			t = this->getTile(i,j);
			if(!t->empty() && t->p->side != side && t->p->is(KING) && validMove(t->p,loc,this)){
				return true;
			}
	}
	return false;
}

char Board::otherSide(char side){
	return (side == WHITE)? BLACK : WHITE;
}





string Board::generateFEN(){
	string output = "";
	Tile t;
	for(int i = 8;i>0;i--){ 
		for(int j = 8;j>0;j--){
			t = this->getTile(i,j);
			if(t->empty()){
				output += EMPTY_SPACE;
			}else{
				output += t->p->FEN;			
			}
		}	
		if(i>1)
			output = output + "/";
	}
	return output;	
}

string Board::getCastleData(){
	string out = ((b->bCastle >> 1)&1)?"bk":"";
	out += (b->bCastle & 1)?"bq":"";
	out += ((b->wCastle >> 1)&1)?"wk":"";
	out += (b->wCastle & 1)?"wq":"";
	return out;
}

string Board::getBoardData(){
	JSON data = createJSON("string");
	string history = NULL;
	addss(data,"FEN",generateFEN(b));
	addss(data,"Castle",getCastleData(b));
	addss(data,"Taken",(char*)&(b->taken));
	addss(data,"Special",b->special);
	size_t length = vector_length(b->history);
	if(length>0){
		history = concat(history,(char*)vector_get(b->history,0),FALSE);
	}
	for(size_t i = 1;i<length;i++){
		history = concat(history,(char*)vector_get(b->history,i),FIRST);
	}
	addss(data,"History",history);
	return jsonToString(data);
}

char Board::numToCol(int c){
	return (char)(c + 96);
}




