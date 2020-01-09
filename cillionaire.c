#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MSG_UNKNOWN "Unknown option."
#define MSG_NOFILE "No file provided. Please start the program with the argument -f [file name]."
#define MAX_QUESTIONS 750

enum category_enum {ent_books, ent_film, ent_music, ent_musicals, ent_tv, ent_vgames, ent_bgames, ent_comics, ent_anime, ent_cartoons, gen_knowledge, science_nature,
science_gadgets, science_computers, science_math, mythology, sports, geography, history, politics, art, celebrities, animals, vehicles};

enum difficulty_enum {easy, medium, hard};

struct game_question
{
    char question[128];
    char answers[4][64];
    enum category_enum ctg;
    enum difficulty_enum diff;
};

void print_menu(void);
int rand_number(void);
void read_questions_file(char *, struct game_question *);

int main(int agrc, char **argv)
{
    char user_input, file_name[100];
    bool player_seed = false, file_provided = false, active = true;
    struct game_question questions[MAX_QUESTIONS];

    if(agrc > 1)
    {
        for (int i = 0; i < agrc; i++)
        {
            if(argv[i][0] == '-')
            {
                if(argv[i][1] == 'f')
                {
                    strcpy(file_name, argv[i + 1]);
                    file_provided = true;
                }
                else if(argv[i][1] == 's')
                {
                    srand(atoi(argv[i + 1]));
                    player_seed = true;
                }
            }
        }
    }

    if(!player_seed)
    {
        srand(time(NULL));
    }

    if (!file_provided)
    {
        puts(MSG_NOFILE);
        return 0;
    }
    else
    {
        read_questions_file(file_name, questions);
        for (unsigned short i = 0; i < 5; i++)
        {
            printf("Question %d: %s\n", i, questions[i].question);
            for (unsigned short j = 0; j < 4; j++)
            {
                printf("Option %d: %s\n", j, questions[i].answers[j]);
            }
            printf("Category enum: %d\n", questions[i].ctg);
            printf("Category diff: %d\n", questions[i].diff);
        }
        return 0;
    }


    print_menu();
    while (active)
    {
        scanf(" %c", &user_input);

        if(user_input == '\n')
            continue;

        switch (user_input)
        {
            case 'q':
                active = false;
                break;
            case 'h':
                print_menu();
                break;
            default:
                puts(MSG_UNKNOWN);
                break;
        }
    }

}

void trim_string(char * s1)
{
	const unsigned short length = strlen(s1);
	char a;
	bool end = false;

	for (unsigned short i = 0; i < length; i++)
	{
		a = *(s1 + i);

		if (a == '\n')
		{
			end = true;
		}

		if (end)
		{
			*(s1 + i) = '\0';
		}
	}
}

bool strings_are_equal(char * s1, char * s2)
{
	const unsigned short length = strlen(s1);
	char a, b;

	for (unsigned short i = 0; i < length; i++)
	{
		a = *(s1 + i);
		b = *(s2 + i);

		if (a != b)
		{
			return false;
		}
	}

	return true;
}

enum difficulty_enum get_difficulty_enum_from_string(char * s)
{
    unsigned short i = 0;
    char diff_literal[3][7] = {"easy", "medium", "hard"};

    for (i = 0; i < 3; i++)
    {
        if (strings_are_equal(s, diff_literal[i]))
        {
          	return i;
        }
    }

    return -1;
}

enum category_enum get_category_enum_from_string(char * s)
{
    unsigned short i = 0;
    char categories_literal[24][128] = {"Entertainment: Books", "Entertainment: Film", "Entertainment: Music", "Entertainment: Musicals and Theatres", "Entertainment: Television",
    "Entertainment: Video Games", "Entertainment: Board Games", "Entertainment: Comics", "Entertainment: Japanese Anime and Manga", "Entertainment: Cartoon and Animations",
    "General Knowledge", "Science and Nature", "Science: Gadgets", "Science: Computers", "Science: Mathematics", "Mythology", "Sports", "Geography", "History", "Politics",
    "Art", "Celebrities", "Animals", "Vehicles"};

    for (i = 0; i < 24; i++)
    {
        if (strings_are_equal(s, categories_literal[i]))
        {
          return i;
        }
    }

    return -1;
}

void set_value_to_string_after_equal(char * phrase, char * value)
{
	bool found_equal = false;
	memset(value, 0, strlen(value));
	for (unsigned short i = 0, j = 0; i < strlen(phrase); i++)
    {
        if (!found_equal && *(phrase + i) == '=')
        {
            found_equal = true;
            continue;
        }

        if (found_equal)
        {
            *(value + j) = *(phrase + i);
            j++;
        }
    }
}

bool string_start_compare(char * string, char * value)
{
    for (unsigned short i = 0; i < strlen(value); i++)
    {
      if (string[i] != value[i])
          return false;
    }

    return true;
}

void read_questions_file(char * file_name, struct game_question * q)
{
	unsigned short option_index = 0, question_count = 0;
	char line[256];
	bool finished_question = false;
	FILE * f = fopen(file_name, "r");

	if (f == NULL)
	{
		fprintf(stdout, "*** Não foi possivel abrir o ficheiro %s.", file_name);
		exit(1);
	}

	while (!feof(f))
	{
		struct game_question question;

		if (finished_question)
		{
			*(q + question_count) = question;
			question_count++;
			finished_question = false;
		}

		if (fgets(line, 256, f) != NULL)
		{
			if (string_start_compare(line, ";"))
			{
				continue;
			}

			trim_string(line);
			if (string_start_compare(line, "QUESTION"))
			{
				set_value_to_string_after_equal(line, question.question);
			}
			else if (string_start_compare(line, "OPTION"))
			{
				set_value_to_string_after_equal(line, question.answers[option_index]);
				option_index++;
				if (option_index == 4)
					option_index = 0;
			}
			else if (string_start_compare(line, "CATEGORY"))
			{
				char literal_value[128];
				set_value_to_string_after_equal(line, literal_value);
				question.ctg = get_category_enum_from_string(literal_value);
			}
			else if (string_start_compare(line, "DIFFICULTY"))
			{
				char literal_value[128];
				set_value_to_string_after_equal(line, literal_value);
				question.diff = get_difficulty_enum_from_string(literal_value);
				finished_question = true;
			}
		}
	}

	fclose(f);
}

int rand_number(void)
{
    return (rand() % 4);
}

void print_menu(void)
{
    puts("********************************************");
    puts("***             CILLIONAIRE                *");
    puts("********************************************");
    puts("*** n <name>     - new game                *");
    puts("*** q            - quit                    *");
    puts("*** h            - show this menu          *");
    puts("*** r <filename> - resume game             *");
    puts("*** s <filename> - save progress and quit  *");
    puts("*** j 50         - play 50:50 joker        *");
    puts("*** j 25         - play 25:75 joker        *");
    puts("*** c            - show credits            *");
    puts("********************************************");
}