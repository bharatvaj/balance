#include <stdio.h>
#include <book.h>

/* TEST_INPUT
october-2023.txt
TEST_INPUT */

int main(int argc, char* argv[]) {
	char* content = (char*)malloc(10000);
	size_t content_len = 0;
	//if (argc < 2) {
	//	fprintf(stderr, "Usage: %s filename.txt\n", argv[0]);
	//	return 1;
	//}
	//FILE* fp = fopen(argv[1], "r");
	FILE* fp = fopen("october-2023.txt", "r");
	if (fp == NULL) {
		printf("Cannot open file\n");
		goto export_csv_cleanup;
	}
	size_t content_read = 0;
	char* content_ptr = content;
	while ( (content_read = fread(content_ptr, 1, 1, fp)) != 0) {
		content_ptr += content_read;
	}
	content_len = content_ptr - content;

	printf("%s\n%ld", content, content_len);

	ledger_parse_data(content, content_len);

export_csv_cleanup:
	if (fp != NULL) fclose(fp);
	return 0;
}

/* TEST_OUTPUT
"2015/10/12","","Exxon","Liabilities:MasterCard","$","-10","",""
"2015/10/12","","Exxon","Expenses:Auto:Gas","$","10","",""
"2015/10/12","","Donna's Cake World","Liabilities:Credit","$","-5","",""
"2015/10/12","","Donna's Cake World","Expenses:Food:Dessert","$","5","",""
"2024/10/25","","Zaitoon","Liabilities:Credit","$","-5","",""
"2024/10/25","","Zaitoon","Expenses:Food:Dinner","$","5","",""
"2024/10/24","","Donna's Cake World","Expenses:Food:Dessert","$","5","",""
"2024/10/24","","Donna's Cake World","Expenses:Food:Dinner","$","19","",""
"2024/10/24","","Donna's Cake World","Assets:Cash","$","-24","",""
"2015/10/12","","Zoho","Income:Salary","$","-10000","",""
"2015/10/12","","Zoho","Assets:Bank:Checking","$","10000","",""
"2025/10/20","","Old loan from friend","Assets:Cash","$","200","",""
"2025/10/20","","Old loan from friend","Friend:Cash","$","-200","",""
TEST_OUTPUT */
