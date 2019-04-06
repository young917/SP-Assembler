#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


// -------------------------- DEFINE NUMBER -----------------------------------

#define TRUE 1
#define FALSE 0

//Define constant number

#define command_num 13
#define reg_num 10
#define MAX_PATH 260
#define MAX_COMMAND 500
#define MEM_SIZE 1048576
#define MEM_LIMIT 1048575
#define HASH_TABLE_SIZE 20
#define SYMBOL_TABLE_SIZE 23



/* Define instruction code

   | Nothing   00 | format1 00 | Nothing 000 |
   | Opcode    01 | format2 01 | r1      001 |
   | Directive 10 | format3 10 | m       010 |
   | Symbol    11 | format4 11 | n       011 |
   |			  |			   | r1, n   100 |
   |		      | 		   | r1, m   101 |
   |		      |	     	   | r1, r2  110 | */
   
#define _INST_INFO 96
#define  _NOTHING 0
#define _OPCODE 32
#define _DIRECTIVE 64
#define _SYMBOL 96

#define _INST_FORMAT 24
#define _FORMAT_1 0
#define _FORMAT_2 8
#define _FORMAT_3 16
#define _FORMAT_4 24

#define _INST_OPERAND 7
#define _NO_OPERAND 0
#define _ONE_REG 1
#define _ONE_MEM 2
#define _ONE_DEC 3
#define _ONE_REG_ONE_DEC 4
#define _TWO_REG 5

enum Command{
	Help, Dir, Quit, History, Dump, Edit, Fill, Reset, Opcode, Opcodelist, Assemble, Type, Symbol, Etc
};
enum MODE{
	Store_input, Erase_space
};
enum CHARTYPE{
	ENTER = 1, COMMA, CHAR, BLANK
};
enum DIRECTIVE{
	START=1, END, BYTE, WORD, RESB, RESW, BASE, NOBASE
};
enum Instruction{
	Nothing = -1, Directive, Format1, Format2, Format3, Format4
};
enum ObjectFile{
	H, T, E, M
};

//-----------------------------   STRUCTURE  ----------------------------------

//  Structure for storing past command
typedef struct input{
	char  str[MAX_COMMAND];
	struct input *next;
}input;

//  Structure for storing (mnemonic,opcode)
typedef struct opcode_info{
	char mnemonics[7];
	unsigned char opcode;
	unsigned int type;	
	struct opcode_info *next;
}opcode_info;

//  Structure for symbol
typedef struct symbol_info{
	char name[7];
	unsigned int address;
	struct symbol_info *next;
}symbol_info;

// Structure for error-list
typedef struct error{
	int line;
	char message[100];
	struct error *next;
}error;

// Structure for assembler
typedef struct Assemble_Info{

	int Line_num;
	unsigned int LOCCTR;
	unsigned int PC;
	unsigned char NIXBPE[6];
	unsigned int Inst_type;
	unsigned int Inst_num;
	char Output[100];

	struct Flags{
		int start;
		int end;
		int error;
		int base;
		int success;
	}Flags;

}Assemble_Info;
	
// Structure for Object code Writer
typedef struct object_program{
	char Output[100];
	char code[70];
	int enter_flag;
	int current_col;
	int modify_num;
	unsigned int modify_record[500];
}object_program;


//--------------------------------  INITIALIZE  -------------------------------   
void Init();
int Hash_func(char mnemonics[], int table_size);
void Make_hash_table();
int Get_Command();

//-----------------------------   Execute command -----------------------------

void help(); 
void show_files(); //dir
void quit();
void show_history(); //history
void mem_dump();
void mem_edit();
void mem_fill();
void mem_reset();
void opcode();
void opcodelist();
void show_content();
void assemble();
void show_symtab();

//----------------------------------  ASSEMBLE  -------------------------------

int assem_pass1( char filename[] );
unsigned int Is_Directive ( char *instruction , char *operand, unsigned int *inc );
unsigned int Is_Mnemonic( char *mnemonic , char *operand, unsigned int *inc );

void assem_pass2(char filename[], int code_len );
int Make_Object_Code( char operand[], char object_code[]);
int Make_Displ( unsigned int *displ, char operand[]);
int Is_Reg( char *operand);
int Is_Dec( char *operand );
void Get_Token( char line[], char store[], int start_idx, int end_idx);

void write_interm( int arg_num, char comment[], char *symbol,  char *instruction, char *operand);
void Get_Info_From_Interm_File( FILE *fp, char lst_line[], char operand[]);

void Write_lst( char line[], unsigned int LOCCTR, int len, int ENTER );
void Write_Obj( int flag, unsigned int addr, char object_code[]);

void Output_File_Initialize( char filename[] );
void initial_reg( char *reg[] );

void find_opcode( char mnemonic[], unsigned int *opcode, unsigned int *inst_type);
int push_symbol( char *symbol, unsigned int LOCCTR );
int Find_Symbol( char *symbol, unsigned int *target_addr );
void erase_symtab();

void Push_into_error_list( char * mes);
void show_error_list();

//----------------------------  Manage input string  -----------------------------

int Str_convert_into_Hex( char str[], unsigned int *num);
void Hex_convert_into_Str( unsigned int num, int len);
int Handling_Input( int mode, char input_str[], char str[], int len, int HEXA);
int Get_argument( unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, int arg_len[]);
int Get_String_Argument( char filename[]);
void Write_Hex(FILE *fp, unsigned int num, int len);
void Write_Blank(FILE *fp, int len);
void Make_Hexa_String( unsigned int num, int len, char str[]);

void store_input();
void erase_symtab();
void end_program();
