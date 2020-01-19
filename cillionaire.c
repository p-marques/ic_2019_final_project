#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MSG_UNKNOWN "Unknown option."
#define MSG_BYE "*** Sad to see you go..."
#define MSG_NOQUESTIONS "*** This is embarrassing but we\u2019re out of questions."
#define MSG_NOFILE "No file provided. Please start the program with the argument -f [file name]."
#define MSG_CORRECT "*** Hooray!"
#define MSG_INCORRECT "*** Woops... That's not correct."
#define MSG_YOUWIN "*** This is incredible! You have won!"
#define MSG_YOULOSE "*** Sorry, you have lost the game. Bye!"
#define MSG_ILLEGALMOVE "*** Illegal move"
#define MSG_CANTSTART "*** Can't start a new game because one is already underway."
#define MSG_SAVEDGAME "*** Ok, your progress has been saved. See you later!"
#define MSG_CANTSAVE "*** No game is underway so can't save!"

enum category_enum {ent_books, ent_film, ent_music, ent_musicals, ent_tv, ent_vgames, ent_bgames, ent_comics, ent_anime, ent_cartoons, gen_knowledge, science_nature,
science_gadgets, science_computers, science_math, mythology, sports, geography, history, politics, art, celebrities, animals, vehicles};

enum difficulty_enum {easy, medium, hard};

typedef struct
{
    char question[128];
    char answers[4][64];
    enum category_enum ctg;
    enum difficulty_enum diff;
    int correct_answer_index;
} game_question;

typedef struct _node {
    game_question q;
    struct _node * next;
    struct _node * previous;
} node;

typedef struct _node * link;

typedef struct {
    char name[20];
    bool j50, j25;
    short level_index;
    bool last_answer;
} Player;

void print_menu(void);
void print_credits(void);
void print_player_status(Player *, int);
int rand_number(void);
void read_game_file(Player *, char *, link *, bool);
bool play(Player *, link *, int *, short *);
int add_question(link *, game_question, int);
game_question get_question(link *, enum difficulty_enum);
int rand_number(void);
void start_new_game(Player *);
void show_question(game_question *, bool);
bool handle_player_question_response(Player *, game_question *, int *, char *);
void use_joker(Player *, game_question *);
void save_game(Player *, link *, game_question *);
void load_game(Player *, link *);
bool string_starts_with(char *, char *);
void flush(link *);

int main(int agrc, char **argv)
{
    char user_input, file_name[100];
    bool player_seed = false, file_provided = false, active = true, playing = false;
    int levels[] = {0, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};
    short difficulty_level[] = {0, 0, 0, 1, 1, 2, 2, 2};
    Player current_player;
    link questions_link;
    game_question current_question;

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

    print_menu();
    while (active)
    {
        printf(">");
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
            case 'c':
                print_credits();
                break;
            case 'n':
                if (playing)
                    puts(MSG_CANTSTART);
                else
                {
                    if (file_provided)
                    {
                        read_game_file(&current_player, file_name, &questions_link, false);
                    }
                    else
                    {
                        puts(MSG_BYE);
                        return 0;
                    }
                    start_new_game(&current_player);
                    current_question = get_question(&questions_link, difficulty_level[current_player.level_index]);
                    show_question(&current_question, true);
                    playing = true;
                }
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
                if (!playing)
                    puts(MSG_UNKNOWN);
                else
                {
                    active = handle_player_question_response(&current_player, &current_question, levels, &user_input);
                }

                if (active)
                {
                    current_question = get_question(&questions_link, difficulty_level[current_player.level_index]);
                    show_question(&current_question, true);
                }

                break;
            case 'j':
                if (!playing)
                    puts(MSG_UNKNOWN);
                else
                    use_joker(&current_player, &current_question);
                break;
            case 's':
                    if(playing)
                    {
                        save_game(&current_player, &questions_link, &current_question);
                        active = false;
                    }
                    else
                    {
                        puts(MSG_CANTSAVE);
                    }
                break;
            case 'r':
                if (playing)
                    puts(MSG_UNKNOWN);
                else
                {
                    load_game(&current_player, &questions_link);
                    print_player_status(&current_player, levels[current_player.level_index]);
                    current_question = get_question(&questions_link, difficulty_level[current_player.level_index]);
                    show_question(&current_question, false);
                    playing = true;
                }
                break;
            default:
                puts(MSG_UNKNOWN);
                break;
        }

        if (!active && playing)
            flush(&questions_link);
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
    char holder;
	bool found_equal = false;
	memset(value, 0, strlen(value));

    for (unsigned short i = 0, j = 0; i < strlen(phrase) + 1; i++)
    {
        if (found_equal)
        {
            holder = *(phrase + i);
            *(value + j) = holder;
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

void write_question(FILE * file, link * last_question, char * question_line, int question_count, bool save_flag)
{
    char line[256], tmp[128];
    game_question working_question;

    trim_new_line(question_line);
    set_value_to_string_after_equal(question_line, working_question.question);

    // Defaulting to 0
    working_question.correct_answer_index = 0;

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

        if (save_flag)
            working_question.ctg = atoi(tmp);
        else
            working_question.ctg = get_category_enum_from_string(tmp);
    }

    if(fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);

        if (save_flag)
            working_question.diff = atoi(tmp);
        else
            working_question.diff = get_difficulty_enum_from_string(tmp);
    }

    if (save_flag && question_count == 0)
    {
        if(fgets(line, 256, file) != NULL)
        {
            trim_new_line(line);
            set_value_to_string_after_equal(line, tmp);

            working_question.correct_answer_index = atoi(tmp);
        }
    }


    add_question(last_question, working_question, question_count);
}

void write_player_details(FILE * file, Player * p, char * name_line)
{
    char line[256], tmp[128];

    trim_new_line(name_line);
    set_value_to_string_after_equal(name_line, p->name);

    if (fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);

        p->level_index = atoi(tmp);
    }

    if (fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);

        p->j50 = atoi(tmp);
    }

    if (fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);

        p->j25 = atoi(tmp);
    }

    if (fgets(line, 256, file) != NULL)
    {
        trim_new_line(line);
        set_value_to_string_after_equal(line, tmp);

        p->last_answer = atoi(tmp);
    }
}

void read_game_file(Player * p, char * file_name, link * questions_link, bool save_flag)
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
            else if (string_starts_with(line, "Player_Name") && save_flag)
            {
                write_player_details(f, p, line);
            }
            else if (string_starts_with(line, "QUESTION"))
            {
                write_question(f, &last_question, line, question_count, save_flag);
                if (question_count == 0)
                {
                    *questions_link = last_question;
                }
                question_count++;
            }
		}
	}

    // DEBUG
    printf("Found %d questions!\n", question_count);

	fclose(f);
}

void start_new_game(Player * p)
{
    char default_name[] = "Newbie";

    fgets(p->name, 20, stdin);
    trim_new_line(p->name);
    if(strlen(&(*p->name)) == 0)
    {
        for (unsigned short i = 0; i < strlen(default_name); i++)
            p->name[i] = default_name[i];
    }
    else
        trim_leading_white_space(p->name);

    // Defaults
    p->last_answer = true;
    p->j25 = true;
    p->j50 = true;
    p->level_index = 0;

    printf("*** Hi %s, let's get started!\n", p->name);
    print_player_status(p, 0);
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

void show_question(game_question * q, bool shuffle_correct_answer)
{
    int correct_answer_index = 0;

    if (shuffle_correct_answer)
    {
        correct_answer_index = rand_number();
        move_correct_answer(q, correct_answer_index);
        q->correct_answer_index = correct_answer_index;
    }

    printf("*** Question: %s\n", q->question);

    for (short i = 0, k = 65; i < 4; i++, k++)
    {
        if (strlen(q->answers[i]) > 0)
            printf("*** %c: %s\n", (char)k, q->answers[i]);
    }
}

bool handle_player_question_response(Player * p, game_question * q, int * levels, char * user_answer)
{
    bool keep_playing = true, player_is_correct = (3 + (*user_answer - 'D') == q->correct_answer_index);

    if (player_is_correct)
    {
        puts(MSG_CORRECT);
        p->level_index++;
        p->last_answer = true;
    }
    else
    {
        puts(MSG_INCORRECT);
        printf("*** The correct answer was %c: %s\n", (char)('A' + q->correct_answer_index), q->answers[q->correct_answer_index]);
        p->level_index = p->level_index > 0 ? p->level_index - 1 : 0;
        keep_playing = p->last_answer;
        p->last_answer = false;
    }

    // player wins
    if (p->level_index == 8)
    {
        puts(MSG_YOUWIN);
        printf("*** Congratulations %s\n", p->name);
        keep_playing = false;
    }
    else
    {
        print_player_status(p, levels[p->level_index]);
    }


    if (!player_is_correct && !keep_playing)
    {
        puts(MSG_YOULOSE);
    }

    return keep_playing;
}

int count_answers(game_question * q)
{
    short count = 0;

    for (unsigned short i = 0; i < 4; i++)
    {
        if ( strlen(q->answers[i]) > 0 )
            count++;
    }

    return count;
}

void remove_answers(game_question * q, short number_of_answers_to_remove)
{
    int r = 0, already_removed = -1;

    while ( number_of_answers_to_remove > 0 )
    {
        r = rand_number();

        // don't remove the correct answer
        if ( r != q->correct_answer_index && r != already_removed )
        {
            for (short i = 0; i < 4; i++)
            {
                if (i == r)
                {
                    q->answers[i][0] = '\0';
                    already_removed = i;
                    number_of_answers_to_remove--;
                }
            }
        }
    }
}

void use_joker(Player * p, game_question * q)
{
    char buffer[40];
    int buffer_n;

    fgets(buffer, 40, stdin);
    trim_new_line(buffer);
    trim_leading_white_space(buffer);
    buffer_n = atoi(buffer);

    // Illegal moves
    if ((count_answers(q) < 4) || (buffer_n == 50 && !p->j50) || (buffer_n == 25 && !p->j25))
    {
        puts(MSG_ILLEGALMOVE);
        return;
    }

    remove_answers(q, buffer_n == 50 ? 2 : 1);

    p->j50 = buffer_n == 50 ? false : p->j50;
    p->j25 = buffer_n == 25 ? false : p->j25;

    for (short i = 0, k = 65; i < 4; i++, k++)
    {
        if (strlen(q->answers[i]) > 0)
            printf("*** %c: %s\n", (char)k, q->answers[i]);
    }
}

void load_game(Player * p, link * questions_link)
{
    char file_name[40];

    fgets(file_name, 40, stdin);
    trim_new_line(file_name);
    trim_leading_white_space(file_name);

    read_game_file(p, file_name, questions_link, true);
    printf("***  Ok %s, where were we? Oh there you go:\n", p->name);
}

void save_game(Player * p, link * questions, game_question * current_question)
{
    FILE * file;
    char file_name[40];
    link aux;

    fgets(file_name, 40, stdin);
    trim_new_line(file_name);
    trim_leading_white_space(file_name);

    file = fopen(file_name, "w");
    if (file == NULL)
	{
		fprintf(stdout, "*** Não foi possivel abrir o ficheiro %s.", file_name);
		exit(1);
	}

    // Player Info
    fputs(";Player Status\n", file);
    fprintf(file, "Player_Name=%s\n", p->name);
    fprintf(file, "Player_Level_Index=%d\n", p->level_index);
    fprintf(file, "Player_J50=%d\n", p->j50);
    fprintf(file, "Player_J25=%d\n", p->j25);
    fprintf(file, "Player_Last_Question=%d\n", p->last_answer);

    // Current question
    fputs(";Questions\n", file);
    fprintf(file, "QUESTION=%s\n", current_question->question);
    for (unsigned short i = 0; i < 4; i++)
    {
        fprintf(file, "OPTION%d=%s\n", i, current_question->answers[i]);
    }
    fprintf(file, "CATEGORY=%d\n", current_question->ctg);
    fprintf(file, "DIFFICULTY=%d\n", current_question->diff);
    fprintf(file, "ANSWER=%d\n", current_question->correct_answer_index);

    // All the other questions
    for (aux = *questions; aux != NULL ; aux = aux->next)
    {
        fprintf(file, "QUESTION=%s\n", aux->q.question);
        for (unsigned short i = 0; i < 4; i++)
        {
            fprintf(file, "OPTION%d=%s\n", i, aux->q.answers[i]);
        }
        fprintf(file, "CATEGORY=%d\n", aux->q.ctg);
        fprintf(file, "DIFFICULTY=%d\n", aux->q.diff);
    }

    puts(MSG_SAVEDGAME);
    fclose(file);
}

int rand_number(void)
{
    return (rand() % 4);
}

void print_player_status(Player * p, int level)
{
    int holder;
    char yes[] = "YES                             *";
    char no[] = "NO                              *";

    puts("********************************************");
    printf("*** Name:  %s", p->name);
    holder = 44 - 11 - strlen(p->name);
    for (unsigned short i = 0; i < holder - 1; i++)
    {
        printf("%c", ' ');
        if ( i == holder - 2 )
            puts("*");
    }

    printf("*** Level: %d", level);

    holder = 44 - 11;

    if (level == 0)
        holder--;
    else
    {
        while (level != 0)
        {
            level /= 10;
            holder--;
        }
    }

    for (unsigned short i = 0; i < holder - 1; i++)
    {
        printf("%c", ' ');
        if ( i == holder - 2 )
            puts("*");
    }

    printf("*** j50:   %s\n", p->j50 == true ? yes : no);
    printf("*** j25:   %s\n", p->j25 == true ? yes : no);
    puts("********************************************");
}

void print_credits(void)
{
    puts("****************************************");
    puts("*** Developer: Pedro \"pMarK\" Marques *");
    puts("****************************************");
}

void print_menu(void)
{
    puts("********************************************");
    puts("***              CILLIONAIRE               *");
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
    bool found = false;
    game_question x;
    link aux;

    for (aux = *questions; aux != NULL; aux = aux->next)
    {
        if (aux->q.diff == diff)
        {
            x = aux->q;

            // Patching link
            if (aux->previous != NULL)
                aux->previous->next = aux->next;
            if (aux ->next != NULL)
                aux->next->previous = aux->previous;

            // If aux->previous == NULL --> removing first element
            if (aux->previous == NULL)
                *questions = aux->next;

            found = true;
            break;
        }
    }

    if (found)
    {
        free(aux);
        return x;
    }
    else
    {
        puts(MSG_NOQUESTIONS);
        exit(0);
    }
}

void flush(link * questions_link)
{
    link aux = *questions_link;
    link holder;

    while (aux != NULL)
    {
        holder = aux->next;
        free(aux);
        aux = holder;
    }
}