#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json/json.h>

void myprintf(char * buffer , int length){
  for(int i = 0; i < length; i++) printf("%c" , buffer[i]);
  printf("\n");
}

struct json_object *parsed_json;
struct json_object *dataFileName;
struct json_object *indexFileName;
struct json_object *recordLength;
struct json_object *keyEncoding;
struct json_object *keyStart;
struct json_object *keyEnd;
struct json_object *order;


struct CharIndex {
	char * key;
	int index;	
};


typedef struct CharIndex IndexCHR;

void Open() {
    	FILE *fp;
	char buffer[1024];
	
	fp = fopen("test.json","r");
	fread(buffer, 1024, 1, fp);
	fclose(fp);

	parsed_json = json_tokener_parse(buffer);

    	json_object_object_get_ex(parsed_json, "dataFileName", &dataFileName);
	json_object_object_get_ex(parsed_json, "indexFileName", &indexFileName);
    	json_object_object_get_ex(parsed_json, "recordLength", &recordLength);
	json_object_object_get_ex(parsed_json, "keyEncoding", &keyEncoding);
    	json_object_object_get_ex(parsed_json, "keyStart", &keyStart);
	json_object_object_get_ex(parsed_json, "keyEnd", &keyEnd);
    	json_object_object_get_ex(parsed_json, "order", &order);


    	printf("datafilename: %s\n", json_object_get_string(dataFileName));
	printf("indexfilename: %s\n", json_object_get_string(indexFileName));
    	printf("recordlength: %d\n", json_object_get_int(recordLength));
	printf("keyencoding: %s\n", json_object_get_string(keyEncoding));
    	printf("keystart: %d\n", json_object_get_int(keyStart));
    	printf("keyend: %d\n", json_object_get_int(keyEnd));
    	printf("order: %s\n", json_object_get_string(order));
}
int compareIndexCHRASC(const void* a, const void* b){       ////  FOR CHR
	return (strcmp(((IndexCHR*)a)->key, ((IndexCHR*)b)->key));
}
int compareIndexCHRDESC(const void* a, const void* b){     ////   FOR CHR
    	return -(strcmp(((IndexCHR*)a)->key, ((IndexCHR*)b)->key));    
}
void CreateIndexFileCHR(IndexCHR * indexes , int RECORD_COUNT){
    
    if(strcmp(json_object_get_string(order),"ASC") == 0)
    {
        qsort(indexes, RECORD_COUNT, sizeof(IndexCHR), compareIndexCHRASC);        
    }
    else{
        qsort(indexes, RECORD_COUNT, sizeof(IndexCHR), compareIndexCHRDESC);
    }

    FILE* fp1;
    fp1 = fopen(json_object_get_string(indexFileName), "w");
    if(!fp1)
	return;
    fseek(fp1, 0, SEEK_SET);
    fwrite(indexes, sizeof(IndexCHR), RECORD_COUNT, fp1);
	fclose(fp1);
}
void CreateIndexFileCHR2(){
    if(strcmp(json_object_get_string(keyEncoding), "CHR")==0)
    { 
        FILE *file = fopen ( json_object_get_string(dataFileName), "r" );

        if ( file != NULL )
        {
            
        fseek(file,0,SEEK_END);
        int filesize = ftell(file);
        fseek(file,0,SEEK_SET);
        printf("record count : %d",filesize/json_object_get_int(recordLength));
        IndexCHR indexes[filesize/json_object_get_int(recordLength)];  /// filesize/json_object_get_int(recordLength) === RECORD_COUNT

        for(int j=0 ; j<filesize/json_object_get_int(recordLength);j++)
        {
            char record [ json_object_get_int(recordLength) ]; /* record char array */   
            fread(record,json_object_get_int(recordLength),1,file);
            char key[json_object_get_int(keyEnd)-json_object_get_int(keyStart)];
            indexes[j].key = malloc(sizeof(char)*filesize/json_object_get_int(recordLength));
            for(int i=json_object_get_int(keyStart);i<json_object_get_int(keyEnd);i++)
            {        
            indexes[j].key[i-json_object_get_int(keyStart)]=record[i];
            }
            indexes[j].index = j;   
        }
        CreateIndexFileCHR(indexes,filesize/json_object_get_int(recordLength));    

        fclose ( file );
        }
        else
        {
        perror ( json_object_get_string(dataFileName) ); /* why didn't the file open? */
        }

    }
}
int BinaryFileSearchCHR(FILE* fp, const char RECORD[22], int firstIdx, int lastIdx){
	printf("\nRunning Binary Search %d %d\n", firstIdx, lastIdx);
	IndexCHR first, last, middle;
	int returnData;
	
	// Calculate the middle Index
	int middleIdx = (firstIdx+lastIdx)/2;

	// Read first record and return if it is the searched one.
	fseek(fp, firstIdx*(sizeof(IndexCHR)), SEEK_SET);
	fread(&first, sizeof(IndexCHR), 1, fp);
	if(strcmp(first.key,RECORD) == 0)
	{
		returnData=first.index;
		return returnData;
	}
	// Read last record and return if it is the searched one.
	fseek(fp, lastIdx*sizeof(IndexCHR), SEEK_SET);
	fread(&last, sizeof(IndexCHR), 1, fp);
	if(strcmp(last.key,RECORD) == 0)
	{
		returnData=last.index;
		
		return returnData;
	}
	// Recursion exit condition, If middle index is equal to first or last index
	// required comparisons are already made, so record is not found.
	// Create and return an empty person.
	if(middleIdx==firstIdx || middleIdx == lastIdx) {
		int d=-1;
		return d;
	}

	// Read the middle record and return if it is the searched one.
	fseek(fp, middleIdx*sizeof(IndexCHR), SEEK_SET);
	fread(&middle, sizeof(IndexCHR), 1, fp);
	if(strcmp(middle.key,RECORD) == 0)
	{
		returnData=middle.index;
		return returnData;
	}
	// Determine the record position and recursively call with appropriate attributes.
	if(strcmp(middle.key,RECORD)>0) {
		return BinaryFileSearchCHR(fp, RECORD, firstIdx+1, middleIdx-1);
	} 
	else {
		return BinaryFileSearchCHR(fp, RECORD, middleIdx+1, lastIdx-1);
	}
}
int FindRecordCHR(char RECORD[22]) {
    // Open the file
    FILE* inputFile;
    inputFile = fopen("data.ndx", "rb");

    // Calculate initial first and last indexes.
    int firstIdx = 0;
    fseek(inputFile, 100*sizeof(IndexCHR), SEEK_SET);
    int lastIdx = (ftell(inputFile)/sizeof(IndexCHR))-1;

    // Initiate the search.
    int result = BinaryFileSearchCHR(inputFile, RECORD, firstIdx, lastIdx);
    fclose(inputFile);
    return result;
}
void PrintIndexFileCHR()
{
	
	FILE* inputFile;
    	inputFile = fopen("data.ndx","rb");
	fseek(inputFile, 0, SEEK_SET);
	int i;	    
	for(i=0;i<100;i++)
        {
		IndexCHR bufferPerson;
		fread(&bufferPerson,sizeof(IndexCHR),1,inputFile);
		printf("name is %s, index is %d \n", bufferPerson.key, bufferPerson.index);
		
	}
	fclose(inputFile);
}

void FindData(int index)
{
   FILE *file = fopen ( json_object_get_string(dataFileName), "r" );
   fseek(file , json_object_get_int(recordLength) * (index) , SEEK_SET);
   char record[json_object_get_int(recordLength)];
   fread(record , json_object_get_int(recordLength) , 1 ,file);
   myprintf(record,64);

}

void printMenu(int * answer){

	//print the user menu
	printf("Welcome \n");
	printf("(1) OPEN\n");
	printf("(2) CREATE INDEX \n");
	printf("(3) SEARCH\n");
	printf("(4) CLOSE \n");
	printf("Please Select one... \n");
	scanf("%d",answer);
}


void main(int argc, char **argv)
{

    int answer;
	
	repeat:
	printMenu(&answer);

	while(answer>4 || answer<1)
	{
		printf("\nEnter a valid choice by pressing ENTER key again");
		printMenu(&answer);
	}

	switch(answer)
	{
		case 1:
            Open();
			goto repeat;
		    break;
        
        case 2:
            CreateIndexFileCHR2();
            PrintIndexFileCHR();
			goto repeat;
            break;

		case 3: printf("Enter the name that you want to search :");
			char search[22];
			scanf("%s", search);
            printf("%s",search);
            FindData(FindRecordCHR(search));
            goto repeat;
			break;
					
		case 4: printf("Program is terminating \n");
			break;
	}



 
   
    
}
