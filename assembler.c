#include "20171697.h"
#include "variable.h"

//---------------- ASSEMBLE MAIN ----------------------------
void assemble(){
	char filename[100];
	char lstname[100];
	int type, arg_num, len;
	int code_len;


	type = Get_String_Argument( filename );
	if( Success == FALSE || type != ENTER){
		Success = FALSE;
		return;
	}

	// Initailize
	ASBL.Flags.start = -1;
	ASBL.Flags.end = FALSE;
	ASBL.Flags.error = FALSE;
	ASBL.LOCCTR = 0;
	ASBL.Line_num = 5;	
	erase_symtab();

	// Pass1
	code_len = assem_pass1(filename);
	
	if( ASBL.Flags.error == TRUE ){
		show_error_list();
		
	//	remove(ASBL.Output);

		erase_symtab();
		Success = FALSE;

		return;
	}

	// Initialize
	ASBL.Flags.end = FALSE;
	ASBL.Flags.base = -1;
	ASBL.LOCCTR = 0;
	ASBL.Line_num = 5;	
	initial_reg( reg );
	OBJ.Output[0] = '\0';
	OBJ.current_col = 0;
	OBJ.modify_num = 0;


	// Pass2
	assem_pass2(filename, code_len );
	
	if( ASBL.Flags.error == TRUE ){
		show_error_list();

	//	remove(ASBL.Output);
	//	remove(OBJ.Output);

		erase_symtab();
		Success = FALSE;
		return;
	}
}


// ---------------------- PASS 1 ------------------------------
int assem_pass1( char filename[] ){
	//return error_flag

	FILE *fp;
	FILE *fp2;
	char lst_name[100];
	char *extend = ".lst";

	char *token[3];
	char line[200];
	char comment[200];
	char *blank = "\0";
	int arg_num;
	char *pt;
	char *tmp;

	unsigned int value = 0;
	unsigned int inst_type;

	int len, i, ret;


	fp = fopen(filename,"r");
	if( fp == NULL ){
		Success = FALSE;
		ASBL.Flags.error = TRUE;
		return 0;
	}

	strcpy( ASBL.Output, filename );
	strcat( ASBL.Output, ".txt" );
	fp2 = fopen( ASBL.Output, "w" );
	fclose( fp2 );

	while(TRUE){

		pt = fgets(line, 200, fp);

		if(pt == NULL)// FILE END
			break;
		comment[0] = '\0';
		strcpy( comment, line );


		// Tokenize
		arg_num = 0;
		token[arg_num] = strtok( line, "\t\n ");
		while ( token[arg_num] != NULL ){

			// make operand ' A  ,  B ' -> ' A,B '
			if ( arg_num > 1 && token[arg_num][0] == ',' ){
				strcat( token[arg_num-1], token[arg_num] );
				arg_num--;
			}
			len = strlen( token[arg_num] );
			if( len >= 2 && token[arg_num][len-1] == ','){
				tmp = strtok( NULL, "\t\n ");
				if( tmp != NULL )
					strcat( token[arg_num], tmp);
			}

			arg_num++;
			if( arg_num == 3 )
				break;
			token[arg_num] = strtok( NULL, "\t\n ");
		}

		// Comment
		if( token[0] != NULL && token[0][0] == '.' ){// COMMENT
			write_interm( 0, comment, blank, blank, blank );
			ASBL.Line_num += 5;
			continue;
		}
		
		// Blank Line
		else if( arg_num == 0 ){
			ASBL.Line_num += 5;
			continue;
		}

		// Error
		else if( ASBL.Flags.end == TRUE ){
			Push_into_error_list( "After END, there exists more instructions.");
			break;
		}

		tmp = strtok( NULL, "\t\n ");
		if( (tmp != NULL) ){// Excess : token more than 3

			Push_into_error_list( "Improper number of columns");
			ASBL.Line_num += 5;

			continue;
		}

		
		// Without Symbol instruction check
		inst_type = Is_Mnemonic( token[0], token[1] , &value );
		if( (inst_type & _INST_INFO) == _NOTHING )
			inst_type = Is_Directive( token[0], token[1] , &value );

		if( (inst_type & _INST_INFO) != _NOTHING ){

			// error
			if( ASBL.Flags.start == -1 ){// START doesn't exist
				Push_into_error_list ( "Code must begin with START.");
				ASBL.Flags.start = 0;
			}

			else if( arg_num > 2 )// Too much Operand
				Push_into_error_list ( "This line has too many arguments.");

			else if( ( inst_type & _INST_INFO ) == _SYMBOL )// Need Symbol
				Push_into_error_list( " This line must have Label.");

			// Write
			else{
				write_interm( arg_num, comment, blank, token[0], token[1]);
				ASBL.LOCCTR += value;
			}
		}

		// With Symbol
		else{

			// Store SYMBOL
			ret = push_symbol( token[0] , ASBL.LOCCTR );
			if( ret == FALSE )
				Push_into_error_list( "fail to push into SYMTAB.");

			else{

				// Instruction Check
				inst_type = Is_Mnemonic( token[1], token[2], &value);
				if( ( inst_type & _INST_INFO ) == _NOTHING )
					inst_type = Is_Directive( token[1], token[2] , &value);

				// error
				if( ( inst_type & _INST_INFO ) == _NOTHING )
					Push_into_error_list("It is not mnemonic or directive.");
			
				else if( ASBL.Flags.start == -1){
					Push_into_error_list("Code must begin with START.");
					ASBL.Flags.start = 0;
				}

				else if( ASBL.Flags.end == TRUE )
					Push_into_error_list("END must not have symbol.");

				// Write
				else{
					write_interm(arg_num, comment, token[0], token[1], token[2]);
					ASBL.LOCCTR += value;

				}
			}
		}
		ASBL.Line_num += 5;
	}

	if( ASBL.Flags.end == FALSE )// Error
		Push_into_error_list( "not proper end of file.");
	
	fclose(fp);

	// Return Code Length
	return (int)( ASBL.LOCCTR - ASBL.Flags.start);

}

unsigned int Is_Directive ( char *instruction , char *operand, unsigned int *inc ){

	char *start = "START";
	char *byte = "BYTE";
	char *word = "WORD";
	char *resb = "RESB";
	char *resw = "RESW";
	char *end = "END";
	char *base = "BASE";
	char *nobase = "NOBASE";

	int state, len, i;
	unsigned int value;
	unsigned int ret = _NOTHING;
	
	if( strcmp( instruction, start ) == 0 ){// START

		if( operand == NULL){
			*inc = 0;
			value = 0;
		}
		else{
			state = Str_convert_into_Hex(operand, &value);

			if( state == FALSE){
				Push_into_error_list("operand has wrong character.");	
			}
			else
				*inc = value;
		}

		state = push_symbol( instruction, value );
		if( state == FALSE )
			Push_into_error_list("fail to make SYMTAB.");

		ASBL.Flags.start = (int)value;
		ASBL.Inst_type = _DIRECTIVE;
		ASBL.Inst_num = START;
		return _DIRECTIVE; 
	}
	
	else if( strcmp( instruction, end) == 0 ){// END
		*inc = 0;
		ASBL.Flags.end = TRUE;
		ASBL.Inst_type = _DIRECTIVE;
		ASBL.Inst_num = END;
		return _DIRECTIVE;
	}
	
	if( operand == NULL){
		Push_into_error_list( "operand must exist.");
		return _NOTHING;
	}

	else if( strcmp( instruction, base) == 0 ){// BASE
		*inc = 0;
		ASBL.Inst_type = _DIRECTIVE;
		ASBL.Inst_num = BASE;
		return _DIRECTIVE;
	}
	else if( strcmp( instruction, nobase) == 0 ){// NOBASE
		*inc = 0;	
		ASBL.Inst_type = _DIRECTIVE;
		ASBL.Inst_num = NOBASE;
		return _DIRECTIVE;
	}
		
	else if( strcmp( instruction, byte) == 0 ){// BYTE
		len = strlen( operand );
		if( operand[1] != '\'' || operand[len-1] != '\'' ){
			Push_into_error_list("Improper operand.");
			ret = _NOTHING;
		}
		else if( operand[0] == 'C'){// char
			*inc = ( len - 3 );
			ret = _SYMBOL;
		}	
		else if( operand[0] == 'X'){// hexa
			len -= 3;
			len = ( len + 1 ) / 2;
			*inc = len;
			ret = _SYMBOL;
		}
		else{
			Push_into_error_list("Improper operand.");
			ret = _NOTHING;
		}
		ASBL.Inst_num = BYTE;
		ASBL.Inst_type = ret;
		return ret;
	}

	else if( strcmp( instruction, word ) == 0 ){// WORD
		*inc = 3;
		ASBL.Inst_type = _SYMBOL;
		ASBL.Inst_num = WORD;
		return _SYMBOL;
	}

	else if( ( strcmp( instruction, resb ) && strcmp( instruction, resw )) != 0 ){// mnemonic error
		return _NOTHING;
	}
	else{
		len = strlen(operand);
		value = 0;
		state = TRUE;
		for( i = 0; i < len ; i++ ){
			if( operand[i] >= '0' && operand[i] <= '9' ){
				value *= 10;
				value += (unsigned int)( operand[i] - '0' );
			}
			else{
				state = FALSE;
				break;
			}
		}			
		if( state == FALSE ){//error
			Push_into_error_list("operand has wrong character.");
			return _NOTHING;
		}

		else if( strcmp( instruction , resb) == 0 ){// RESB
			*inc= value;
			ASBL.Inst_num = RESB;
			ASBL.Inst_type = _SYMBOL;
			return _SYMBOL;
		}
		else if( strcmp( instruction , resw) == 0 ){// RESW
			*inc = ( 3 * value );
			ASBL.Inst_num = RESW;
			ASBL.Inst_type = _SYMBOL;
			return _SYMBOL;
		}
	}
}

unsigned int Is_Mnemonic( char *mnemonic , char *operand, unsigned int *inc ){

	char revise_mne[10];
	char blank[2];

	int format;
	int flag_form4 = FALSE;

	unsigned int inst_type = _NOTHING;
	unsigned int opcode;
	unsigned int operand_need;

	if( mnemonic[0] == '+' ){
		strcpy( revise_mne, mnemonic+1 );
		flag_form4 = TRUE;
	}
	else
		strcpy( revise_mne, mnemonic );
		
	find_opcode( revise_mne, &opcode , &inst_type);

	// Not in OPTAB
	if( inst_type == _NOTHING )
		return inst_type;

	// Not format4
	if( ( ( inst_type & _FORMAT_3 ) == 0 ) && ( flag_form4 == TRUE ) ){
		Push_into_error_list("This mnemonic doesn't have format 4.");
		return inst_type;
	}
	

	operand_need = inst_type & _INST_OPERAND;
	
	// Operand Number error
	if( ( operand_need == _NO_OPERAND ) && ( operand != NULL ) ){
		Push_into_error_list("This mnemonic doesn't have operand");
		return inst_type;		
	}
	else if( ( operand_need != _NO_OPERAND ) && ( operand == NULL )){
		Push_into_error_list( "This mnemonic must have operand.");
		return inst_type;
	}

	switch( inst_type & _INST_FORMAT ){
		
		case _FORMAT_1: *inc = 1; break;
		case _FORMAT_2: *inc = 2; break;
		case _FORMAT_3: *inc = 3; break;
		case _FORMAT_4: *inc = 4; break;
	}

	ASBL.Inst_num = opcode;
	ASBL.Inst_type = inst_type;

	return inst_type;
}

//---------------------------  PASS 2   ---------------------------

void assem_pass2(char filename[], int code_len ){

	FILE *fp;
	FILE *fp2;
	char *pt;
	char interm_file_name[100];

	char lst_line[38];
	char operand[10];
	char buffer[7];
	char object_code[200];

	unsigned int addr;
	
	int len, i, j, ret;

	// File Open

	strcpy( interm_file_name, ASBL.Output );
	Output_File_Initialize( filename );

	fp = fopen( interm_file_name , "r" );
	if( fp == NULL ){		
		Push_into_error_list("Intermediate file doesn't exist.");
		return;
	}

	// Get PC
	fgets( buffer, 5, fp );
	Str_convert_into_Hex( buffer, &ASBL.PC );

	
	while(TRUE){

		Get_Info_From_Interm_File( fp, lst_line, operand); 
		
		if( ( ASBL.Inst_type & _INST_INFO) == _DIRECTIVE ){

			// START
			if( ASBL.Inst_num == START ){

				// Write List File
				Write_lst( NULL, ASBL.LOCCTR, 4, FALSE );
				Write_lst( lst_line, 0, 0, TRUE );

				// Write Object File
				Get_Token( lst_line, object_code, 4, 10 );// program name
			   	Make_Hexa_String( ASBL.Flags.start, 6, object_code + 7 );// start adr.
				Make_Hexa_String( code_len, 6, object_code + 13 );// code len.

				Write_Obj( H, ASBL.LOCCTR, object_code );
				continue;
				
			}

			// END
			else if( ASBL.Inst_num == END ){
				
				if( operand[0] == ' ' )
					addr = ASBL.Flags.start;

				else{
					pt = strtok( operand, " ");
					Find_Symbol( pt, &addr );
				}
				Write_lst("    ", 0, 0, FALSE );
				Write_lst( lst_line, 0, 0, TRUE);

				Write_Obj( E, addr , NULL );
				break;
			}
		}

		// Make Object Code
		ret = Make_Object_Code( operand , object_code );
		
		if( ret == FALSE){// error
			Push_into_error_list( "Improper operand.");
			continue;
		}

		// Write Output File
		if( ASBL.PC - ASBL.LOCCTR == 0 )
			Write_lst( "    ", 0, 0, FALSE );

		else
			Write_lst( NULL, ASBL.LOCCTR, 4, FALSE );

		Write_lst( lst_line, 0, 0, FALSE );
		Write_lst( object_code, 0, 0, TRUE );

		Write_Obj( T, ASBL.LOCCTR, object_code );
	}

	fclose( fp );

	/* TEST
	remove( interm_file_name );*/
}

// ---------------------- Intermediate File -------------------------------

void write_interm( int arg_num, char comment[], char *symbol,  char *instruction, char *operand){
	
	/* 	  LOCCTR   Line num            Symbol            (+)Instruction            (@,#)    Operand           Opcode
	   |  4space |  4space  | 4space | 6space | 4space |     7space     | 3space | 1space | 9space | 4space | 8space | 

	   [0]       [4]        [8]      [12]     [18]     [22]             [29]     [32]     [33]     [42]     [46]  
	*/

	int len;
	FILE *fp;
	char flag;

	if( ASBL.Flags.error == TRUE )
		return;

	fp = fopen( ASBL.Output , "a");

	//Write LOCCTR
	Write_Hex(fp, ASBL.LOCCTR, 4);

	//Write Comment
	if( arg_num == 0 ){
		fprintf(fp, ".   " );
		fprintf(fp, "%s", comment);
		fclose(fp);
		return;
	}

	//Write Line Num
	Write_Hex(fp, ASBL.Line_num , 4);
	Write_Blank(fp, 4);

	//Write SYMBOL
	if( symbol != NULL ){
		fprintf(fp, "%s", symbol);
		len = 10 - strlen(symbol);
	}
	else
		len = 10;
	Write_Blank(fp, len);

	//Write instruction
	if( instruction != NULL ){
		fprintf(fp, "%s", instruction);
		len = 10 - strlen( instruction );
	}
	else
		len = 10;
	Write_Blank(fp, len);

	//Write Operand
	if( operand != NULL ){
		len = strlen(operand);
		flag = operand[0];
		if(flag == '#' || flag == '@'){
			fprintf(fp, "%s", operand);
			len = 14 - len;
		}
		else{
			fprintf(fp, " %s", operand);
			len = 13 - len;
		}
	}
	else
		len = 14;
	Write_Blank(fp, len );

	// Write instruction code
	Write_Hex( fp, ASBL.Inst_type , 2);

	// Write Dir_num or Opcode
	Write_Hex( fp, ASBL.Inst_num , 1);

	fprintf(fp, "\n");

	fclose(fp);

}

void Get_Info_From_Interm_File( FILE *fp, char lst_line[], char operand[]){

	char buffer[5];
	char comment[200];
	char *pt;
	int i;

	// Comment
	while( TRUE ){
		fgets( buffer, 5, fp );
		if( buffer[0] != '.' )
			break;
		fgets( comment, sizeof(comment), fp );
		Write_lst( comment, 0, 0 , FALSE);
	}
	
	// Line number
	Str_convert_into_Hex( buffer, &ASBL.Line_num );
	
	// Locctr
	ASBL.LOCCTR = ASBL.PC;
	
	// One line
	fgets( lst_line, 38 , fp );

	// Set NIXBPE
	for( i = 0 ; i < 6; i++ )
		ASBL.NIXBPE[i] = 0;

	if( lst_line[28] == '@' )
		ASBL.NIXBPE[0] = 1;

	else if( lst_line[28] == '#' )
		ASBL.NIXBPE[1] = 1;

	else{
		ASBL.NIXBPE[0] = 1;
		ASBL.NIXBPE[1] = 1;
	}

	// Operand
	Get_Token( lst_line, operand, 25, 34 );

	// Inst_type  & Inst_num
	fgets( comment, fp );
	buffer[0] = comment[0];
	buffer[1] = comment[1];
	buffer[2] = '\0';
	Str_convert_into_Hex( buffer, &ASBL.Inst_type );

	buffer[0] = comment[2];
	buffer[1] = '\0';
	Str_convert_into_Hex( buffer, &ASBL.Inst_num );

	// PC
	pt = fgets( buffer, 5, fp );
	if( pt == NULL )// File End
		return;

	Str_convert_into_Hex( buffer, &ASBL.PC );
}

//---------------------- Write  Output File ---------------------------

void Write_lst( char line[], unsigned int LOCCTR, int len, int ENTER ){

	FILE *fp;
	fp = fopen( ASBL.Output, "a" );

	if( len != 0 )
		Write_Hex( fp, LOCCTR, len);

	else
		fprintf( fp, "%s", line );
	
	if( ENTER == TRUE )
		fprintf( fp, "\n" );	
	
	fclose(fp);

}

void Write_Obj( int flag, unsigned int addr, char object_code[]){
	
	FILE *fp;
	int len, num;
	int column;
	char code[10];
	unsigned int tmp;
	char address[7];
	char length[2];
	int i;

	fp = fopen( OBJ.Output, "a" );

	switch( flag ){
		case H: {
					object_code[19] = '\0';
					fprintf(fp, "H%s\n",object_code);
					break;
				}
		case T:{
				   len = strlen( object_code );
				   column = OBJ.current_col;

				   if( ( column != 0 ) && (column + len <= 69 ) ){
					   strcat( OBJ.code + column , object_code );
					   column += len;
				   }
				   else if( column != 0 ){
					   OBJ.code[column] = '\0';
					   Make_Hexa_String( column - 9, 2, length );
					   OBJ.code[8] = length[0];
					   OBJ.code[9] = length[1];
					   fprintf( fp, "%s\n", OBJ.code);
					   column = 0;
				   }

				   if( column == 0 ){
					   code[0] = 'T';
					   Make_Hexa_String( addr, 6, OBJ.code + 1 );
					   strcpy( OBJ.code + 9, object_code );
					   OBJ.current_col = len + 9;
				   }
				   break;
			   }

		case E:					
		case M:{
				   num = OBJ.modify_num;
				   for( i = 0; i < num ; i ++ ){
					   Make_Hexa_String( OBJ.modify_record[i], 8, code );
					   fprintf(fp, "M%s\n", code);
				   }
				   
				   Make_Hexa_String( addr , 6, address );
				   fprintf( fp, "E%s", address );
			   }
	}
	
	fclose(fp);

}


//---------------------------- Making Code -----------------------------------

int Make_Object_Code( char operand[], char object_code[]){
	// return error or not

	int inst_info;
	int inst_format;
	int inst_operand;
	unsigned int inst_num = ASBL.Inst_num;
	unsigned int displ;

	char *reg[reg_num];
	char *operand1, *operand2;

	int ret = TRUE;
	int val, val2;
	int i, j, len;
	int remain; 

	inst_info = ASBL.Inst_type & _INST_INFO;
	
	if( inst_info == _OPCODE ){
		
		inst_format = ASBL.Inst_type & _INST_FORMAT;
		inst_operand = ASBL.Inst_type & _INST_OPERAND;

		switch( inst_format ){

			case _FORMAT_1:{
							  len = 2;
							  break;
						  }	 
			case _FORMAT_2:{
							  inst_num = inst_num << 4;

							  operand1 = strtok( operand, "," );
						      operand2 = strtok( NULL, "," );

							  if( operand1 == NULL ){
								  ret = FALSE;
								  break;
							  }
							  
							  switch( inst_operand ){

								  case _ONE_REG:{
													i = Is_Reg( operand1 );

													if( i == reg_num )
														ret = FALSE;

													else if( operand2 != NULL )
														ret = FALSE;
													
													else{
														inst_num += i;
														inst_num = inst_num << 4;
													}
													break;
												}
								  case _ONE_DEC:{
													val = Is_Dec( operand1 );											
													if( 0<= val && val <= 0xF && operand2 == NULL ){
														inst_num += val;
														inst_num = inst_num << 4;
														ret = TRUE;
													}
													else
														ret = FALSE;
													break;
												}

								  case _TWO_REG:{
													if( operand1 == NULL || operand2 == NULL )
														ret = FALSE;

													else{
														val = Is_Reg( operand1 );
														val2 = Is_Reg( operand2 );

														if( val == reg_num || val2 == reg_num )
															ret = FALSE;

														else{
															inst_num += val;
															inst_num = inst_num << 4;
															inst_num += val2;
															ret = TRUE;
														}
													}
													break;
												}
								  case _ONE_REG_ONE_DEC:{
															if( operand1 == NULL || operand2 == NULL )
																ret = FALSE;

															else{
																val = Is_Reg( operand1 );
																val2 = Is_Dec( operand2 );

																if( val == reg_num ){
																	ret = FALSE;
																	break;
																}
																else if( val > 0xF ){
																	ret= FALSE;
																	break;
																}
																inst_num += val;
																inst_num = inst_num << 4;
																inst_num += val2;
															}
														}
							  }

							  len = 4;
							  break;
						  }
			case _FORMAT_3:{

							 if( inst_operand != _NO_OPERAND ){

								 ret = Make_Displ( &displ, operand );

								 if( ret != FALSE ){

									 // opcode 6bit
									 inst_num = inst_num >> 2;

									 // write nixbpe
									 for( i = 0; i < 6 ; i ++ ){
										 inst_num = inst_num << 1;
										 if( ASBL.NIXBPE[i] == 1 ){
											 inst_num += 1;
										 }
									 }
									 inst_num = inst_num << 12;

									 // write displ
									 inst_num += displ;
								 }
							 }
							 else
								 inst_num = inst_num << 16;	

							 len = 6;
							 break;
						  }	
			case _FORMAT_4:{
							  ASBL.NIXBPE[5] = 1;
							  
							  if( inst_operand != _NO_OPERAND ){

								 ret = Make_Displ( &displ, operand );

								 if( ret != FALSE ){

									 // opcode 6 bit
									 inst_num = inst_num >> 2;

									 // write nixbpe
									 for( i = 0; i < 6 ; i ++ ){
										 inst_num = inst_num << 1;
										 if( ASBL.NIXBPE[i] == 1 ){
											 inst_num += 1;
										 }
									 }
									 inst_num = inst_num << 20;
									 
									 //write displ;
									 inst_num += displ;
							 	 }
							  }
							  else
								 inst_num = inst_num << 24;	

							  len = 8;
							  break;
						  }

		}

		Make_Hexa_String( inst_num, len , object_code );
	}

	// Directive
	else if( inst_info == _DIRECTIVE ){

		object_code[0] = '\0';

		switch( inst_num ){

			case BASE:{
						  ret = Find_Symbol( operand, &ASBL.Flags.base );
						  break;
					  }

			case NOBASE:{
							ASBL.Flags.base = -1;
							break;
						}

			case BYTE:{
						  len = strlen( operand );

						  j = 0;
						  if( ( operand[0] == 'X' ) && ( len % 2 == 1 ) ){
							  object_code[j] = 0;
							  j++;
						  }
						  for( i = 2 ; i < (len - 1 ) ; i++ ){

							  // C'   ' case
							  if( operand[0] == 'C' ){
								  val = (int)operand[i];
								  Make_Hexa_String( val, 2, object_code + j);
								  j += 2;
							  }

							  // X'    ' case
							  else if( operand[i] >= '0' && operand[i] <= '9' ){
								  object_code[j] = operand[i];
								  j++;								 
							  }
							  else if( operand[i] >= 'A' && operand[i] <= 'F' ){
								  object_code[j] = operand[i];
								  j++;
							  }
							  else{
								  ret = FALSE;
								  break;
							  }
						  }
						  object_code[j] = '\0';
						  break;
					  }
			case WORD:{
						  val = Is_Dec( operand );
						  if( val > 0xFFFFFF )
							  ret = FALSE;

						  else if( val == -1 )
							  ret = FALSE;

						  else
							  Make_Hexa_String( val , 6, object_code );
					  }
		}
	}
	return ret;
}


int Make_Displ( unsigned int *displ, char operand[] ){

	char *operand1, *operand2;
	unsigned int address;
	unsigned int tmp_displ;
	int minus;
	int ret = FALSE;
	int i, len;
	int val = -1;

	operand1 = strtok( operand, "," );
	operand2 = strtok( NULL, "r" );

	if( operand1 != NULL){ 

		// X flag
		if( operand2 != NULL && strcmp( operand2,"X") == 0 ){
			ASBL.NIXBPE[2] = 1;
		}
		else if( operand2 != NULL ){
			ret = FALSE;
			return ret;
		}

		// IMEDDIATE
		if( ASBL.NIXBPE[0] == 0 && ASBL.NIXBPE[1] == 1 ){

			// FIND DECIMAL
			val = Is_Dec( operand1 );

		}
		if( val != -1 ){

			// Judge Decimal Excess
			if( ASBL.NIXBPE[5] == 0 && val > 0xFFF )
				ret = FALSE;

			else if( ASBL.NIXBPE[5] == 1 && val > 0xFFFFF )
				ret = FALSE;
			
			// Store Decimal
			else{
				ASBL.NIXBPE[3] = ASBL.NIXBPE[4] = 0;
				*displ = (unsigned int)val;
				ret = TRUE;
			}

			return ret;
		}

		// FIND SYMTAB
		else{

			ret = Find_Symbol( operand, &address );
				
			if( ret != FALSE ){

				// Format 4 and Need Modification
				if( ASBL.NIXBPE[5] == 1 ){
					if( address <= 0xFFFFF ){

						*displ = address;
						
						// Store Modify code
						address = ASBL.LOCCTR - ASBL.Flags.start + 1;
						address = address << 8;
						address += 5;

						val = OBJ.modify_num;
						OBJ.modify_record[val] = address;
						OBJ.modify_num++;
						ret = TRUE;

					}
					else
						ret = FALSE;

					return ret;
				}

				// PC-Relative
				if( address < ASBL.PC ){
					minus = TRUE;
					tmp_displ = ASBL.PC - address;
				}
				else{
					minus = FALSE;
					tmp_displ = address - ASBL.PC;
				}
				if( 0 <= tmp_displ && tmp_displ <= 2047 ){

					if( minus == TRUE ){
						tmp_displ = tmp_displ ^ 0xFFF;
						tmp_displ += 1;
					}
					ASBL.NIXBPE[4] = 1;
					*displ = tmp_displ;
					ret = TRUE;

					return ret;
				}

				// Base_Relative
				if( ASBL.Flags.base >= 0 ){

					if( address >= ASBL.Flags.base ){
						tmp_displ = address - ASBL.Flags.base;

						if( 0 <= tmp_displ && tmp_displ <= 4095 ){
							ASBL.NIXBPE[3] = 1;
							*displ = tmp_displ;
							ret = TRUE;

							return ret;
						}
					}
				}

				// Direct_Address and Need Modified
				if( address <= 0xFFF ){

					*displ = address;
					
					// Store Modify code
					address = ASBL.LOCCTR - ASBL.Flags.start + 1;
					address = address << 8;
					address += 3;

					val = OBJ.modify_num;
					OBJ.modify_record[val] = address;
					OBJ.modify_num++;
					ret = TRUE;

					return ret;

				}
			}
		}
	}
	return ret;
}

int Is_Reg( char *operand ){

	int i;

	for( i = 0 ; i < reg_num ; i++ )
		if( strcmp( operand, reg[i] ) == 0 )
			break;
	return i;
}

int Is_Dec( char *operand ){

	int len, i;
	int val;

	len = strlen( operand );
	
	for( i = 0; i < len ; i++ ){
		if( operand[i] < '0' || operand[i] > '9' ){
			val = -1;
			break;
		}
		val += ( operand[i] - '0');
	}
	
	return val;
}


void Get_Token( char line[], char store[], int start_idx, int end_idx){
	int i;
	int idx = 0;

	for( i = start_idx ; i < end_idx ; i++, idx++ ){
		
		if( line[i] == ' ')
			break;
		
		store[idx] = line[i];
	}
	store[idx] = '\0';	
}

// --------------------  Initialization ---------------------------------

void Output_File_Initialize( char filename[] ){
	
	FILE *fp;
	FILE *fp2;
	int len;

	len = strlen( filename );
	len -= 4;
	filename[len] = '\0';
	
	strcpy( ASBL.Output , filename );
	strcpy( OBJ.Output , filename );
	strcat( ASBL.Output , ".lst" );
	strcat( OBJ.Output , ".obj");
	fp = fopen( ASBL.Output, "w");
	fclose( fp );
	fp2 = fopen( OBJ.Output, "w");
	fclose( fp2 );

}

void initial_reg( char *reg[] ){
	reg[0] = "A";
	reg[1] = "X";
	reg[2] = "L";
	reg[3] = "B";
	reg[4] = "S";
	reg[5] = "T";
	reg[6] = "F";
	reg[8] = "PC";
	reg[9] = "SW";

}


// ----------------- OPTAB ------------------------------

void find_opcode( char mnemonic[], unsigned int *opcode, unsigned int *inst_type){
	int adr;
	int format;
	opcode_info *cur;

	adr = Hash_func(mnemonic);
	cur = Hash_Table[adr];
	while(cur != NULL){
		if( strcmp(cur->mnemonics, mnemonic) == 0)
			break;
		cur = cur->next;
	}

	//Not found
	if( cur == NULL){
		return;
	}

	*opcode = (unsigned int)cur->opcode;
	*inst_type = (unsigned int)cur->type;

	return;
}


// ------------------- SYMTAB ---------------------------

int push_symbol( char *symbol, unsigned int LOCCTR ){

	int ret = TRUE;
	symbol_info *new_sym;
	symbol_info *pre, *cur;
	int Hash_value;
	int idx, len;

	// Is symbol consist of Upper Alphabet?
	len = strlen( symbol );
	for( idx = 0; idx < len; idx++ )
		if( symbol[idx] < 'A' || symbol[idx] > 'Z' )
			return FALSE;

	new_sym = (symbol_info *)malloc( sizeof(symbol_info) );
	if( new_sym == NULL ){
		return FALSE;
	}

	strcpy( new_sym->name, symbol );
	new_sym->address = LOCCTR;
	Hash_value = Hash_func( symbol );

	pre = NULL;
	cur = SYMTAB[Hash_value];

	while( cur != NULL ){
		switch( strcmp(symbol, cur->name) ){
			case 0: ret = FALSE; break;
			case 1: break;
		}
		pre = cur;
		cur = cur->next;
	}
	if( ret == FALSE )
		return ret;

	if( pre == NULL ){
		new_sym->next = SYMTAB[Hash_value];
		SYMTAB[Hash_value] = new_sym;
	}
	else{
		new_sym->next = cur;
		pre->next = new_sym;
	}
	return ret;
}

int Find_Symbol( char *symbol, unsigned int *target_addr ){

	symbol_info *cur;
	int Hash_value;
	int ret = FALSE;
	int state;

	Hash_value = Hash_func( symbol );
	
	cur = SYMTAB[Hash_value];
	while( cur != NULL ){
		state = strcmp( symbol, cur->name);
		if( state == -1 )
			cur = cur->next;
		else if( state == 0 ){
			ret = TRUE;
			*target_addr = cur->address;
			break;
		}
	}
	return ret;
}

void erase_symtab(){
	int idx;
	symbol_info *present;
	symbol_info *next;

	for( idx = 0; idx < SYMBOL_TABLE_SIZE ; idx++ ){
		
		next = SYMTAB[idx];
		present = NULL;
		while( next != NULL ){
			present = next;
			next = present->next;
			free(present);
			present = NULL;
		}

		SYMTAB[idx] = NULL;
	}
}



// ------------------- Error List -------------------------
void Push_into_error_list( char * mes){

	error *new_error;
	
	new_error = (error*)malloc(sizeof(error));
	new_error->line = ASBL.Line_num;
	strcpy( new_error->message, mes);
	new_error->next = NULL;					
	ASBL.Flags.error = TRUE;

	if( Error_list_head == NULL ){
		Error_list_head = new_error;
		Error_list_tail = new_error;
	}
	else{
		Error_list_tail->next = new_error;
		Error_list_tail = new_error;
	}

}

void show_error_list(){// And erase
	error *cur, *pre;

	pre = cur = Error_list_head;
	while( cur != NULL ){
		pre = cur;
		cur = pre->next;
		printf("line	%d	:	%s\n", pre->line, pre->message );
		free(pre);
		pre = NULL;
	}

	Error_list_head = NULL;
}
