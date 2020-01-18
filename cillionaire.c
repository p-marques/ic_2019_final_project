#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MSG_UNKNOWN "Unknown option."
#define MSG_BYE "*** Sad to see you go..."
#define MSG_NOQUESTIONS "*** This is embarrassing but we're out of questions."
#define MSG_NOFILE "No file provided. Please start the program with the argument -f [file name]."
#define MSG_CORRECT "*** Hooray!"
#define MSG_INCORRECT "*** Woops... That's not correct."
#define MSG_YOUWIN "*** This is incredible! You have won!"
#define MSG_YOULOSE "*** Sorry, you have lost the game. Bye!"

enum category_enum {ent_books, ent_film, ent_music, ent_musicals, ent_tv, ent_vgames, ent_bgames, ent_comics, ent_anime, ent_cartoons, gen_knowledge, science_nature,
science_gadgets, science_computers, science_math, mythology, sports, geography, history, politics, art, celebrities, animals, vehicles};

enum difficulty_enum {easy, medium, hard};

typedef struct
{
    char question[128];
    char answers[4][64];
    enum category_enum ctg;
    enum difficulty_enum diff;
} game_question;

typedef struct _node {
    game_question q;
    struct _node * next;
    struct _node * previous;
} node;

typedef struct _node * link;

typedef struct {
    char name[50];
    bool j50, j25;
    short level_index;
    bool last_answer;
} Player;

void print_menu(void);
int rand_number(void);
void read_questions_file(char *, link *);
bool play(Player *, link *, int *, short *);
int add_question(link *, game_question, int);
game_question get_question(link *, enum difficulty_enum);
int rand_number(void);
void start_new_game(Player *);
char show_question(game_question *);
bool handle_player_question_response(Player *, char *, char, char *);

int main(int agrc, char **argv)
{
    char user_input, file_name[100];
    bool player_seed = false, file_provided = false, active = true, playing = false;
    int levels[] = {0, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};
    short difficulty_level[] = {0, 0, 0, 1, 1, 2, 2, 2};
    Player current_player;
    link questions_link;
    game_question current_question;
    char current_question_answer;

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
        read_questions_file(file_name, &questions_link);
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
                puts(MSG_BYE);
                active = false;
                break;
            case 'h':
                print_menu();
                break;
            case 'n':
                start_new_game(&current_player);
                current_question = get_question(&questions_link, difficulty_level[current_player.level_index]);
                current_question_answer = show_question(&current_question);
                playing = true;
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
                if (!playing)
                    puts(MSG_UNKNOWN);
                else
                {
                    active = handle_player_question_response(&current_player, &user_input, current_question_answer, current_question.answers[3 + (current_question_answer - 'D')]);
                }

                if (active)
                {
                    current_question = get_question(&questions_link, difficulty_level[current_player.level_index]);
                    current_question_answer = show_question(&current_question);
                }

                break;
            case 'j':

                break;
            default:
                puts(MSG_UNKNOWN);
                break;
        }
    }

}

void trim_new_line(char * s1)
{
	const unsigned short length = strlen(s1);

	for (unsigned short i = 0; i < length; i++)
	{
		if (*(s1 + i) == '\n')
		{
			*(s1 + i) = '\0';
		}
	}
}

void trim_leading_white_space(char * s)
{
    const unsigned short length = strlen(s);
    unsigned short i = 0, k = 0;

    for (i = 0; i < length; i++)
    {
        if (s[i] != 32)
        {
            k = i;
            break;
        }
    }
    
    for (i = 0; i < length; i++)
    {
        s[i] = s[k];
        k++;
    }
}

enum difficulty_enum get_difficulty_enum_from_string(char * s)
{
    unsigned short i = 0;
    char diff_literal[3][7] = {"easy", "medium", "hard"};

    for (i = 0; i < 3; i++)
    {
        if (strcmp(s, diff_literal[i]) == 0)
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
        if (strcmp(s, categories_literal[i]) == 0)
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
        if (found_equal)
        {
            *(value + j) = *(phrase + i);
            j++;
        }
        else if (*(phrase + i) == '=')
        {
            found_equal = true;
            continue;
        }
    }
}

bool string_starts_with(char * string, char * value)
{
    for (unsigned short i = 0; i < strlen(value); i++)
    {
      if (string[i] != value[i])
          return false;
    }

    return true;
}

void write_question(FILE * file, link * last_question, char * question_line, int question_count)
{
    char line[256], tmp[128];
    game_question working_question;

    trim_new_line(question_line);
    set_value_to_string_after_equal(question_line, working_question.question);

    for (unsigned short i = 0; i < 4; i++)
    {
        if(fgets(line, 256, file) != NULL)
        {
            trim_new_line(line);
            set_value_to_string_after_equal(line, working_question.answers[i]);
        }
    }
    
    if(fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);
        working_question.ctg = get_category_enum_from_string(tmp);
    }

    if(fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);
        working_question.diff = get_difficulty_enum_from_string(tmp);
    }

    add_question(last_question, working_question, question_count);
}

void read_questions_file(char * file_name, link * questions_link)
{
    unsigned int question_count = 0;
	char line[256];
    link last_question;
	FILE * f = fopen(file_name, "r");

	if (f == NULL)
	{
		fprintf(stdout, "*** Não foi possivel abrir o ficheiro %s.", file_name);
		exit(1);
	}

	while (!feof(f))
	{
		if (fgets(line, 256, f) != NULL)
		{
			if (string_starts_with(line, ";"))
			{
				continue;
			}
            else if (string_starts_with(line, "QUESTION"))
            {
                write_question(f, &last_question, line, question_count);
                if (question_count == 0)
                {
                    *questions_link = last_question;
                }
                question_count++;
            }
		}
	}

	fclose(f);
}

void start_new_game(Player * p)
{
    char default_name[] = "newbie";

    fgets(p->name, 50, stdin);
    trim_new_line(p->name);
    if(strlen(&(*p->name)) == 0)
    {
        for (unsigned short i = 0; i < strlen(default_name); i++)
            p->name[i] = default_name[i];
    }
    else
        trim_leading_white_space(p->name);

    // Default last answer to true
    p->last_answer = true;

    printf("*** Hi %s, let's get started!\n", p->name);
}

void move_correct_answer(game_question * q, int correct_answer_index)
{
    char tmp[64];
    
    strcpy(tmp, q->answers[0]);
    for (short i = 0; i < correct_answer_index; i++)
    {
        strcpy(q->answers[i], q->answers[i + 1]);
    }

    strcpy(q->answers[correct_answer_index], tmp);
}

char show_question(game_question * q)
{
    int correct_answer_index = rand_number();
    move_correct_answer(q, correct_answer_index);

    printf("*** Question: %s\n", q->question);

    for (short i = 0, k = 65; i < 4; i++, k++)
    {
        printf("*** %c: %s\n", (char)k, q->answers[i]);
    }

    return (char)(65 + correct_answer_index);
}

bool handle_player_question_response(Player * p, char * user_answer, char correct_answer, char * answer)
{
    bool keep_playing = true, player_is_correct = (*user_answer == correct_answer);

    if (player_is_correct)
    {
        puts(MSG_CORRECT);
        p->level_index++;
        p->last_answer = true;
    }
    else
    {
        puts(MSG_INCORRECT);
        printf("*** The correct answer was %c: %s\n", correct_answer, answer);
        p->level_index = p->level_index > 0 ? p->level_index - 1 : 0;
        keep_playing = p->last_answer;
        p->last_answer = false;
    }

    // player wins
    if (p->level_index == 7)
    {
        puts(MSG_YOUWIN);
        printf("*** Congratulations %s\n", p->name);
        keep_playing = false;
    }

    if (!player_is_correct && !keep_playing)
    {
        puts(MSG_YOULOSE);
    }

    return keep_playing;
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

int add_question(link * last_question, game_question q, int question_count)
{
    link tmp = (node *)malloc(sizeof(node));
    if (tmp == NULL)
    {
        puts("Out of memory!");
        return 0;
    }

    tmp->q = q;
    tmp->next = NULL;
    tmp->previous = NULL;

    if (question_count > 0)
    {
        tmp->previous = *last_question;
        (*last_question)->next = tmp;
    }

    *last_question = tmp;
    return 1;
}

game_question get_question(link * questions, enum difficulty_enum diff)
{
    game_question x;

    for (link aux = *questions; aux != NULL; aux = aux->next)
    {
        if (aux->q.diff == diff)
        {
            x = aux->q;
            aux->previous->next = aux->next;

            free(aux);
            return x;
        }
    }

    puts(MSG_NOQUESTIONS);
    exit(0);
}