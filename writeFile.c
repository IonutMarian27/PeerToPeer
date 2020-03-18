//Creates file with bytes size
//This can be implemented in more than 1 way
void preallocateFile(char *filePath, long long bytes) {
    
}

//Writes pieceLength bytes to filePath beginning from pieceIndex.
void writeToFile(char *filePath, int pieceLength, int pieceIndex, char *bytes) {

}

//Reads pieceLength bytes from file starting at pieceIndex then returns that as char *
char *readFromFile(char *filePath, int pieceLength, int pieceIndex) {

}

//Reads the piece at pieceIndex from filePath, transforms it into SHA1 using openssl/sha.h
//Compare computed SHA1 to initialSHA and return 1-ok, otherwise
//Create error code as you wish.
int verifyIntegrity(char *filePath, int pieceLength, int pieceIndex, char* initialSHA) {

}


//Verify if the data at pieceIndex is equal to initialSHA.
//If it is, it means you have that piece. Otherwise, you don't.
int verifyIfHasPiece(char *filePath, int pieceLength, int pieceIndex, char *initialSHA) {
    return verifyIntegrity(filePath, pieceLength, pieceIndex, initialSHA) == 1 ? 1 : -1;
}