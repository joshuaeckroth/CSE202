#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
using namespace std;

#define READ 0
#define WRITE 1

bool run_test(char *program, string test, string input, string output);
void expand_string_array(string *&str_array, int oldsize, int newsize);
pid_t mypopen(const char *program, int *infp, int *outfp);

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cerr << "*** Usage: " << argv[0] << " <test file> <program file>" << endl;
		return EXIT_FAILURE;
	}

	char *test_file = argv[1];
	char *program_file = argv[2];

	ifstream test_file_stream(test_file, ios::in);
	if(!test_file_stream.good())
	{
		cerr << "*** Test file " << test_file << " cannot be opened." << endl;
		cerr << "*** Unable to proceed." << endl;
		return EXIT_FAILURE;
	}

	int test_count = 0;
	int successful_tests = 0;
	string *tests = NULL;
	string *inputs = NULL;
	string *outputs = NULL;
	string *tmp = NULL;

	string line;
	bool inside_input = false;
	bool inside_output = false;
	while(!test_file_stream.eof())
	{
		getline(test_file_stream, line);
		if(line.substr(0, 3) == "===")
		{
			inside_input = false;
			inside_output = false;

			expand_string_array(tests, test_count, test_count + 1);
			expand_string_array(inputs, test_count, test_count + 1);
			expand_string_array(outputs, test_count, test_count + 1);

			test_count++;

			tests[test_count - 1] = line.substr(4);
		}
		else if(line.substr(0, 9) == "--- Input")
		{
			inside_input = true;
			inside_output = false;
		}
		else if(line.substr(0, 10) == "--- Output")
		{
			inside_input = false;
			inside_output = true;
		}
		else
		{
			if(inside_input)
			{
				inputs[test_count - 1] += line;
				inputs[test_count - 1] += "\n";
			}
			else if(inside_output)
			{
				outputs[test_count - 1] += line;
				outputs[test_count - 1] += "\n";
			}
		}
	}

	cout << endl;
	cout << "** Running " << test_count << " tests..." << endl << endl;
	for(int i = 0; i < test_count; i++)
	{
		if(run_test(program_file, tests[i], inputs[i], outputs[i]))
			successful_tests++;
	}
	double successful_percent = 100 * static_cast<double>(successful_tests) / static_cast<double>(test_count);

	cout << endl;
	cout << "** " << successful_tests << " of " << test_count << " (";
	cout.setf(ios::fixed, ios::floatfield);
	cout.precision(0);
	cout << successful_percent;
	cout << "%) successful." << endl;

	return EXIT_SUCCESS;
}

bool run_test(char *program, string test, string input, string output)
{
	int infp, outfp, status, pos, substr_length, read_length;
	pid_t pid, w;
	const int bufsize = 100;
	char buf[bufsize];
	string program_output, program_output_stripped, output_stripped;
	
	if((pid = mypopen(program, &infp, &outfp)) <= 0)
	{
		cerr << "*** Error executing " << program << endl;
		return false;
	}

	pos = 0;
	while(pos < input.size())
	{
		if((input.size() - pos) > bufsize)
			substr_length = bufsize;
		else
			substr_length = input.size() - pos;

		memset(buf, 0x0, bufsize);

		strncpy(buf, input.substr(pos, substr_length).c_str(), substr_length);
		pos += substr_length;
		write(infp, buf, substr_length);
	}
	close(infp);

	waitpid(pid, &w, 0);
	if(w == -1)
	{
		cerr << "*** Error waiting for program to finish." << endl;
		return false;
	}

	fcntl(outfp, F_SETFL, O_NONBLOCK);

	while((read_length = read(outfp, buf, bufsize)) > 0)
	{
		buf[read_length] = '\0';
		program_output += buf;
	}

	// strip newlines, spaces, and tabs in both program_output and output
	program_output_stripped = program_output;
	output_stripped = output;

	while((pos = program_output_stripped.find(' ')) != -1)
		program_output_stripped.erase(pos, 1);
	while((pos = program_output_stripped.find('\t')) != -1)
		program_output_stripped.erase(pos, 1);
	while((pos = program_output_stripped.find('\n')) != -1)
		program_output_stripped.erase(pos, 1);
	while((pos = program_output_stripped.find('\r')) != -1)
		program_output_stripped.erase(pos, 1);

	while((pos = output_stripped.find(' ')) != -1)
		output_stripped.erase(pos, 1);
	while((pos = output_stripped.find('\t')) != -1)
		output_stripped.erase(pos, 1);
	while((pos = output_stripped.find('\n')) != -1)
		output_stripped.erase(pos, 1);
	while((pos = output_stripped.find('\r')) != -1)
		output_stripped.erase(pos, 1);

	if(program_output_stripped == output_stripped)
	{
		cout << "** Success! " << test << endl;
		return true;
	}
	else
	{
		cout << endl;
		cout << "** Failed " << test << endl;
		cout << endl;
		cout << "--> Desired output (ignore spaces):" << endl << output << endl;
		cout << "--> Program output (ignore spaces):" << endl << program_output << endl;
		cout << endl << endl;
		return false;
	}
}

// from: http://www.unix.com/unix-dummies-questions-answers/63311-how-write-stdin-another-program-program-stdin-program-b.html
pid_t mypopen(const char *program, int *infp, int *outfp)
{
	int p_to_program[2], p_from_program[2];
	pid_t pid;
	
	if(pipe(p_to_program) != 0 || pipe(p_from_program) != 0)
		return -1;

	pid = fork();
	if(pid < 0)
		return pid;
	else if(pid == 0)
	{
		close(p_to_program[WRITE]);
		dup2(p_to_program[READ], READ);
		close(p_from_program[READ]);
		dup2(p_from_program[WRITE], WRITE);
		execl(program, program, NULL);
		cerr << "*** Error executing " << program << endl;
		exit(EXIT_FAILURE);
	}

	if(infp == NULL)
		close(p_to_program[WRITE]);
	else
		*infp = p_to_program[WRITE];
	if(outfp == NULL)
		close(p_from_program[READ]);
	else
		*outfp = p_from_program[READ];
	return pid;
}

void expand_string_array(string *&str_array, int oldsize, int newsize)
{
	string *tmp = str_array;
	str_array = new string[newsize];

	for(int i = 0; i < oldsize; i++)
	{
		str_array[i] = tmp[i];
	}
	delete [] tmp;
}
