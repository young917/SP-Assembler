#include "20171697.h"
#include "variable.h"


//******************Main Function************************
int main (void){
	Init();
	while(!Exit_flag){
		switch(Get_Command()){
			case Help: help(); break;
			case Dir: show_files(); break;
			case Quit: quit(); break;
			case History: show_history(); break;
			case Dump: mem_dump(); break;
			case Edit: mem_edit(); break;
			case Fill: mem_fill(); break;
			case Reset: mem_reset(); break;
			case Opcode: opcode(); break;
			case Opcodelist: opcodelist(); break;
			case Type: show_content(); break;
			case Assemble: assemble(); break;
			case Symbol: show_symtab(); break;
			default: Success = FALSE; break;
		}
		if(Success)
			store_input();
		else{
			free(new_input);
			new_input = NULL;
			printf("Error: invalid input\n");
		}
	}
	end_program();
}

//********************Initialization***********************
void Init(){
	int i;

	Exit_flag = FALSE;
	History_Head = NULL;
	Error_list_tail = Error_list_tail = NULL;

	Memory = (unsigned char *)calloc(MEM_SIZE,sizeof(unsigned char));
	if(Memory == NULL){
		Exit_flag = TRUE;
		printf("Error: fail to allocate memory\n");
		return;
	}
	last_mem_idx = -1;

	//Store Command//
	command_list[0][0] = "h";
	command_list[0][1] = "help";
	command_list[1][0] = "d";
	command_list[1][1] = "dir";
	command_list[2][0] = "q";
	command_list[2][1] = "quit";
	command_list[3][0] = "hi";
	command_list[3][1] = "history";
	command_list[4][0] = "du";
	command_list[4][1] = "dump";
	command_list[5][0] = "e";
	command_list[5][1] = "edit";
	command_list[6][0] = "f";
	command_list[6][1] = "fill";
	command_list[7][0] = "reset";
	command_list[7][1] = "\0";
	command_list[8][0] = "opcode";
	command_list[8][1] = "\0";
	command_list[9][0] = "opcodelist";
	command_list[9][1] = "\0";
	command_list[10][0] = "assemble";
	command_list[10][1] = "\0";
	command_list[11][0] = "type";
	command_list[11][1] = "\0";
	command_list[12][0] = "symbol";
	command_list[12][1] = "\0";
	
	//Make Hash Table//
	Make_hash_table();

	//Initialize SYMTAB
	for( i = 0; i < SYMBOL_TABLE_SIZE ; i++ )
		SYMTAB[i] = NULL;

}

//************ Relate to Hash Table *********************
int Hash_func(char mnemonics[], int table_size){//Return Hash value
	int idx = 0;
	int total = 0;
	int ret;

	while(1){
		if(mnemonics[idx] == '\0')
			break;
		total += (mnemonics[idx]-'A');
		idx++;
	}

	ret = (total % table_size);

	// keep return value positive
	if(ret < 0)
		ret += table_size;

	return ret;
}
void Make_hash_table(){//Make Hash Table
	char line[500];
	char hex_str[3];
	char format[4];
	FILE *fp;
	int i,j, tmp;
	int store_adr;
	char *ret;
	unsigned int opc;
	unsigned int inst_type = _OPCODE;
	opcode_info *new_info;

	//Initialize Hash Table
	for(i = 0; i < HASH_TABLE_SIZE ; i++)
		Hash_Table[i] = NULL;

	//Open File
	fp = fopen("opcode.txt","r");
	if(fp == NULL){
		printf("Error: cannot read opcode.txt file\n");
		Exit_flag = TRUE;
		return;
	}

	while(1){
		//Read one line of file
		ret = fgets(line,500,fp);
		if(ret == NULL)
			break;

		rd_pt = 0;
		new_info = (opcode_info *)malloc(sizeof(opcode_info));
		if(new_info == NULL){
			printf("Error: fail to allocate memory\n");
			Exit_flag = TRUE;
			break;
		}
		// Get opcode
		Handling_Input( Store_input, line, hex_str, 2, TRUE);// Read
		Str_convert_into_Hex(hex_str, &opc);
		new_info->opcode = (unsigned char)opc;// Store

		// Ignore space
		Handling_Input( Erase_space, line, NULL, 0, FALSE);

		// Get mnemonic
		Handling_Input( Store_input, line, new_info->mnemonics, 6, FALSE);// Read
		store_adr = Hash_func(new_info->mnemonics, HASH_TABLE_SIZE );//Store

		// Ignore space
		Handling_Input( Erase_space, line, NULL, 0 , FALSE);

		// Get format
		Handling_Input( Store_input, line, format, 3, FALSE);
		inst_type = _OPCODE;

		i = 0;
		while( i <= 2){

			if( i == 1 && format[1] == '/' )
				i = 2;
			else if( i == 1 )
				break;

			tmp = format[i] - '0';
			switch( tmp ){
				case 1: {
							inst_type = inst_type | _FORMAT_1;
							inst_type = inst_type | _NO_OPERAND;
							break;
						}

				case 2: {
							inst_type = inst_type | _FORMAT_2;
							if( opc == 0xB4 || opc == 0xB8 )
								inst_type = inst_type | _ONE_REG;

							else if( opc == 0xB0 )
								inst_type = inst_type | _ONE_DEC;

							else if( opc == 0xA4 || opc == 0xA8 )
								inst_type = inst_type | _ONE_REG_ONE_DEC;

							else
								inst_type = inst_type | _TWO_REG;
							break;
						}

				case 3: {
							inst_type = inst_type | _FORMAT_3;
							if( opc == 0x4C )
								inst_type = inst_type | _NO_OPERAND;
							else
								inst_type = inst_type | _ONE_MEM;
							break;
						}

				case 4: {
							inst_type = inst_type | _FORMAT_4;
							if( opc == 0x4C )
								inst_type = inst_type | _NO_OPERAND;
							else
								inst_type = inst_type | _ONE_MEM;
							break;
						}
			}
			i++;
		}
		new_info->type = inst_type;

		//Store in the Hash Table
		new_info->next = Hash_Table[store_adr];
		Hash_Table[store_adr] = new_info;

	}
	fclose(fp);
}

//*******************Identify Command***********************
int Get_Command(){
	int identify_command = Etc;
	int ret = Etc;
	char command[11];

	Success = TRUE;
	new_input = (input*)malloc(sizeof(input));
	if(new_input == NULL){
		printf("Error: fail to allocate memory\n");
		Success = FALSE;
		return identify_command;
	}
	new_input->next = NULL;

	//Get Input
	printf("sicsim> ");
	fgets(new_input->str,MAX_COMMAND,stdin);
	rd_pt = 0;

	//Erase space before command
	ret = Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE);
	if(ret != CHAR){
		Success = FALSE;
		return identify_command;
	}
	
	//Store command
	ret =  Handling_Input( Store_input, new_input->str, command, 10, FALSE);

	//Error
	if( ret == COMMA || ret == FALSE){
		Success = FALSE;
		return identify_command;
	}
	else if( strcmp("\0",command) == 0 ){
		Success = FALSE;
		return identify_command;
	}

	//Find input command 
	for(identify_command = 0; identify_command < command_num ; identify_command++){
		if(!(strcmp(command_list[identify_command][0],command)&&strcmp(command_list[identify_command][1],command))){
			return identify_command;
		}
	}
	//Not Found
	return identify_command;
}

//*********************'help' Command*******************
void help(){
	int idx;
	int len;

	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}

	//Display command list
	for( idx = 0 ; idx < command_num ; idx++ ){
		printf("%s", command_list[idx][0]);
		len = strlen(command_list[idx][0]);
		if( command_list[idx][1][0] != '\0' ){
			printf("[");
			while( command_list[idx][1][len] != '\0' ){
				printf("%c", command_list[idx][1][len]);
				len++;
			}
			printf("]");
		}
		if( idx == Dump ){
			printf(" [start, end]");
		}
		else if( idx == Edit ){
			printf(" address, value");
		}
		else if( idx == Fill ){
			printf(" start, end, value");
		}
		else if ( idx == Opcode ){
			printf(" mnemonic");
		}
		else if ( idx == Assemble || idx == Type ){
			printf(" filename");
		}
		printf("\n");
	}
}

//*********************'dir' Command**********************
void show_files(){
	DIR *current_dir; // directory stream
	struct dirent *dr_st; //dirent structure
	struct stat stat_st; //stat structure
	int file_type;

	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}

	//Open current directory
	if ((current_dir = opendir(".")) == NULL){
		printf("Error: cannot open current directory\n");
		return;
	}

	//Look all files in the current directory
	while(1){
		if((dr_st = readdir(current_dir)) == NULL)
			break;
		stat(dr_st->d_name,&stat_st);
		file_type = stat_st.st_mode & S_IFMT;
		printf("%s",dr_st->d_name);
		if(file_type == S_IFDIR){
			printf("/");
		}
		else if((stat_st.st_mode & S_IXUSR) != 0){//user has permission to execute
			printf("*");
		}
		else if((stat_st.st_mode & S_IXGRP) != 0){//group has permission to execute
			printf("*");
		}
		else if((stat_st.st_mode & S_IXOTH) != 0){//other has permission to execute
			printf("*");
		}
		printf("\n");
	}
	closedir(current_dir);
}

//*********************'quit' Command************************
void quit(){
	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}
	Exit_flag = TRUE;
	return;
}

//*********************'history' Command********************
void show_history(){
	input *cur;
	int num=1;

	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}

	//Print past commands from History_Head to History_Tail
	cur = History_Head;
	while(cur != NULL){
		printf("%d	%s", num, cur->str);
		cur = cur->next;
		num++;
	}
	printf("%d	%s", num, new_input->str);
}

//*********************'dump' Command************************
void mem_dump(){
	unsigned int start_m;
	unsigned int end_m;
	unsigned int tmp;
	unsigned int start_p;
	unsigned int end_p;
	int dump_case = 0;
	int idx;
	int r;
	int arg_len[3];
	arg_len[0] = arg_len[1] = 5;
	arg_len[2] = 2;

	//Store argument and number of arguments
	dump_case = Get_argument(&start_m, &end_m, &tmp, arg_len);
	if( !Success )
		return;
	else if(dump_case == 3){
		Success = FALSE;
		return;
	}

	switch(dump_case){
		case 0:{//# of arguments is zero.
				   start_m = last_mem_idx + 1;
				   if( start_m > MEM_LIMIT)
					   start_m = 0;
				   start_p = 16 * (start_m/16);
				   end_m = 160 + start_m - 1;
				   if( end_m > MEM_LIMIT )
					   end_m = MEM_LIMIT;
				   end_p = 16 * (end_m/16);
				   break;
			   }
		case 1:{//# of arguments is one.
				   start_p = 16 * (start_m/16);
				   end_m = 160 + start_m - 1;
				   if( end_m > MEM_LIMIT )
					   end_m = MEM_LIMIT;
				   end_p = 16 * (end_m/16);
				   break;

			   }
		case 2:{//# of argument is two.
				   if( start_m > end_m ){
					   Success = FALSE;
					   return;
				   }
				   start_p = 16 * (start_m/16);
				   end_p = 16 * (end_m/16); 
			   }
	}
	//print memory
	for( idx = start_p ; idx <= end_p ; idx += 16){
		Hex_convert_into_Str((unsigned int)idx,5);//print address
		printf(" ");
		for( r = idx ; r < (idx + 16) ; r++ ){//print value of memory
			if( (r > end_m) || (r < start_m) )  
				printf("   ");
			else{
				Hex_convert_into_Str( (unsigned int)Memory[r], 2);
				printf(" ");
			}
		}
		printf("; ");
		//print ASCII code of memory
		for( r = idx ; r < (idx + 16) ; r++ ){
			if( (r > end_m) || (r < start_m) )  
				printf(".");
			else if( (Memory[r] >= 32) && (Memory[r] <= 126) )
				printf("%c",Memory[r]);
			else
				printf(".");
		}
		printf("\n");
	}
	last_mem_idx = end_m;//store end memory
}

//*********************'edit' Command************************///
void mem_edit(){
	unsigned int address;
	unsigned int value;
	unsigned int tmp;
	int arg_num;
	int arg_len[3];
	arg_len[0] = 5;
	arg_len[1] = 2;
	arg_len[2] = 0;
	
	//Store arguments and the number of arguments
	arg_num = Get_argument(&address, &value, &tmp, arg_len);
	if( arg_num != 2)
		Success = FALSE;
	else if(Success)
		Memory[address] = (unsigned char)value;
}

//*********************'fill' Command************************///
void mem_fill(){
	unsigned int start_m;
	unsigned int end_m;
	unsigned int value;
	int arg_num;
	int idx;
	int arg_len[3];
	arg_len[0] = arg_len[1] = 5;
	arg_len[2] = 2;

	//Store arguments and the number of arguments
	arg_num = Get_argument(&start_m, &end_m, &value, arg_len);
	if( arg_num != 3)
		Success = FALSE;
	else if(Success){
		if(start_m > end_m)
			Success = FALSE;
		else{
			for( idx = start_m ; idx<= end_m ; idx++ )
				Memory[idx] = (unsigned char)value;
		}
	}
}

//*********************'reset' Command************************
void mem_reset(){
	int idx;

	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}
	//Make all memory zero
	for( idx = 0; idx <= MEM_LIMIT ; idx++)
		Memory[idx] = 0;
}

//*********************'opcode' Command************************
void opcode(){
	int ret;
	int adr;
	opcode_info *cur;
	char mnem[7];

	//Ignore space 
	ret = Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE);
	//Error
	if(ret != CHAR){
		Success = FALSE;
		return;
	}

	//Store mnemonic
	ret = Handling_Input( Store_input, new_input->str, mnem, 6, FALSE);
	//Error
	if(ret == BLANK)
		ret = Handling_Input( Erase_space,new_input->str, NULL, 0, FALSE);
	if(ret != ENTER){
		Success = FALSE;
		return;
	}

	//Find Hash value
	adr = Hash_func(mnem, HASH_TABLE_SIZE );
	//Error
	if( adr < 0 ){
		Success = FALSE;
		return;
	}

	//Find current mnemonic in the Hash Table
	cur = Hash_Table[adr];
	while(cur != NULL){
		if( strcmp(cur->mnemonics, mnem) == 0)
			break;
		cur = cur->next;
	}
	//Not found
	if( cur == NULL){
		Success = FALSE;
		return;
	}
	printf("opcode is ");
	Hex_convert_into_Str((unsigned int)cur->opcode,2);
	printf(".\n");
}

//*********************'opcodelist' Command************************
void opcodelist(){
	int idx;
	opcode_info *cur;

	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}
	//Print opcode table
	for( idx = 0 ; idx < HASH_TABLE_SIZE ; idx++){
		printf("%d : ",idx);
		cur = Hash_Table[idx];
		while(cur != NULL){
			printf("[%s, ",cur->mnemonics);
			Hex_convert_into_Str((unsigned int)cur->opcode,2);
			printf("]");
			cur = cur->next;
			if(cur == NULL){
				printf("\n");
				break;
			}
			printf(" -> ");
		}
	}
}
void show_content(){
	char *ret;
	int type;
	char filename[MAX_COMMAND];
	char line[MAX_COMMAND];
	FILE *fp;
	int file_kind;
	struct stat file_info;  

	type = Get_String_Argument( filename );
	if( Success == FALSE || type != ENTER){
		Success = FALSE;
		return;
	}
	
	stat(filename, &file_info);
	file_kind = file_info.st_mode & S_IFMT;
	if( file_kind == S_IFDIR ){// Directory case: Error
		Success = FALSE;
		return;
	}

	fp = fopen(filename,"r");
	if( fp == NULL ){
		Success = FALSE;
		return;
	}

	while(1){

		ret = fgets( line, MAX_COMMAND, fp );
		if( ret == NULL )
			break;

		printf("%s",line);
	}

	fclose(fp);
}

//*********************Store past Command***********************
void store_input(){
	//If empty Queue
	if(History_Head == NULL){
		History_Head = new_input;
		History_Tail = History_Head;
	}
	//Push into the queue
	else{
		History_Tail->next = new_input;
		History_Tail = new_input;
	}
}

//********************* End Program - Free memory ****************
void end_program(){
	int idx;
	opcode_info *prev;
	opcode_info *current;
	input *cur;
	input *after;
	
	after = cur = History_Head;
	while(after != NULL){
		cur = after;
		after = cur->next;
		free(cur);
		cur = NULL;
	}

	if(Memory != NULL)
		free(Memory);
	Memory = NULL;

	for( idx = 0 ; idx < HASH_TABLE_SIZE ; idx++ ){
		prev = current = Hash_Table[idx];
		while(current != NULL){
			prev = current;
			current = current->next;
			free(prev);
			prev = NULL;
		}
	}

	erase_symtab();
}
