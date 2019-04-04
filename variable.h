//Linked List for past command - Queue
input *History_Head;
input *History_Tail;
input *new_input;

//index that indicates where to start to read input text
int rd_pt;

//Store command list
char* command_list[command_num][2];

//1MB Memory space
unsigned char *Memory;
//index that indicates where ends printing
int last_mem_idx;

//Hash table
opcode_info* Hash_Table[HASH_TABLE_SIZE];

symbol_info* SYMTAB[SYMBOL_TABLE_SIZE];
error *Error_list_head;
error *Error_list_tail;

//For assembler
assemble_flags Assemble_State;
object_program Object_file_State;

//Flag for exit program
int Exit_flag;

//Flag for success input
int Success;
