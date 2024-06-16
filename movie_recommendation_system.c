#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERS 100
#define MAX_MOVIES 10
#define MAX_RATING 10
#define MAX_MOVIE_NAME 200
#define MAX_NAME_LENGTH 100
#define USER_DATA_FILE "user_data.txt"
#define MOVIE_DATABASE_FILE "movie_database.txt"
#define USER_RATINGS_FILE "user_ratings.txt"
#define TEMP_RATING_FILE "temp_rating.txt"

// Function Prototypes
void displayMainMenu();
int registerUser();
int checkUserExists(const char *username);
void displayMovies();
void toLowerCase(char *str);
int userCount();
int updateNewUserRating();
int getUserIndex(const char *username);
void rateMovie();
void recommendMovie();

// Global variables
char user_data[MAX_USERS][MAX_NAME_LENGTH];
int count = 0;
int num_movies = 10;
int userIndex = 0;
char *ratingsLine = NULL;
size_t ratingsLen = 0;
ssize_t readingRatings;
FILE *fptr;

int main()
{
    int choice;

    while (1)
    {
        displayMainMenu();
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            userCount();
            registerUser();
            break;
        case 2:
            displayMovies();
            break;
        case 3:
            rateMovie();
            break;
        case 4:
            recommendMovie();
            break;
        case 0:
            printf("Exiting The Movie Recommendation System\n");
            exit(0);
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

void displayMainMenu()
{
    printf("\n***** Movie Recommendation System *****\n");
    printf("1. Register User\n");
    printf("2. Display Movies\n");
    printf("3. Rate a Movie\n");
    printf("4. Get Movie Recommendations\n");
    printf("0. Exit\n");
    printf("Enter your choice: ");
}

int userCount()
{
    FILE *file = fopen(USER_DATA_FILE, "r");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return 0;
    }

    char username[MAX_NAME_LENGTH];
    while (fgets(username, sizeof(username), file) != NULL)
    {
        count++;
    }
    fclose(file);
    return count;
}

int checkUserExists(const char *username)
{
    char existingUser[MAX_NAME_LENGTH];
    int userId = 1;

    FILE *file = fopen(USER_DATA_FILE, "r");
    if (file == NULL)
    {
        return 0;
    }

    while (fscanf(file, "%s", existingUser) != EOF)
    {
        toLowerCase(existingUser);
        if (strcmp(existingUser, username) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int updateNewUserRating()
{
    int numUsers;
    int numMovies;

    FILE *ratingsFile = fopen(USER_RATINGS_FILE, "r+");
    if (ratingsFile == NULL)
    {
        printf("Error opening ratings file.\n");
        return 0;
    }

    // Read the first line to get the current number of users and movies
    readingRatings = getline(&ratingsLine, &ratingsLen, ratingsFile);
    if (readingRatings == -1)
    {
        printf("Failed to read the ratings file.\n");
        fclose(ratingsFile);
        return 0;
    }
    ratingsLine[strcspn(ratingsLine, "\n")] = 0; // Remove newline character
    numUsers = atoi(strtok(ratingsLine, " "));
    numMovies = atoi(strtok(NULL, " "));

    numUsers++; // Increment the number of users

    // Rewind to the start of the file and update the header
    fseek(ratingsFile, 0, SEEK_SET);
    fprintf(ratingsFile, "%d %d", numUsers, numMovies);

    // Move to the end of the file to add new user ratings
    fseek(ratingsFile, 0, SEEK_END);
    for (int i = 0; i < numMovies; i++)
    {
        fprintf(ratingsFile, i < numMovies - 1 ? "0.0 " : "0.0\n");
    }

    fclose(ratingsFile);
    return 1;
}

int registerUser()
{
    char username[MAX_NAME_LENGTH];

    while (1)
    {
        printf("Enter username for registration: ");
        scanf("%100s", username);

        toLowerCase(username);

        if (!checkUserExists(username))
        {
            FILE *file = fopen(USER_DATA_FILE, "a");
            if (file == NULL)
            {
                printf("Error opening user data file.\n");
                return 0;
            }
            fprintf(file, "%s %d\n", username, count + 1);
            count = 0;
            fclose(file);

            printf("User %s is successfully registered.\n", username);
            updateNewUserRating();
            return 1;
        }
        else
        {
            printf("User already exists. Please choose a different name.\n");
        }
    }
}

void displayMovies()
{
    char movie[MAX_MOVIE_NAME];
    int index = 1;

    FILE *file = fopen(MOVIE_DATABASE_FILE, "r");
    if (file == NULL)
    {
        printf("Error opening movie database file.\n");
        return;
    }

    printf("***** Movie Database *****\n");
    while (fgets(movie, sizeof(movie), file) != NULL)
    {
        printf("%d. %s", index++, movie);
    }

    fclose(file);
}

void toLowerCase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

int getUserIndex(const char *username)
{

    char tempUsername[MAX_NAME_LENGTH];
    int index = 0;

    FILE *file = fopen(USER_DATA_FILE, "r");
    if (file == NULL)
    {
        printf("Unable to open user data file.\n");
        return -1;
    }

    while (fscanf(file, "%s %d\n", tempUsername, &index) != EOF)
    {
        toLowerCase(tempUsername);
        if (strcmp(tempUsername, username) == 0)
        {
            fclose(file);
            return index;
        }
    }

    fclose(file);
    return -1;
}

int updateUserRating(int userId, int movie, float rating)
{

    // Open ratings file and create a temp file to write
    FILE *ratingFile = fopen(USER_RATINGS_FILE, "r");
    FILE *temp = fopen(TEMP_RATING_FILE, "w");

    if (!ratingFile || !temp)
    {
        // Handle error if file opening fails
        perror("Error opening file");
        return -1;
    }

    // Copy lines from the file over to the temp file until we get to the line we wish to edit
    for (int i = 1; i < userId + 1; i++)
    {
        readingRatings = getline(&ratingsLine, &ratingsLen, ratingFile);
        fprintf(temp, "%s", ratingsLine);
    }

    // On the line of interest, copy ratings before the movie index
    readingRatings = getline(&ratingsLine, &ratingsLen, ratingFile);
    char *token = strtok(ratingsLine, " ");
    for (int i = 1; i < movie; i++)
    {
        fprintf(temp, "%s ", token);
        token = strtok(NULL, " ");
    }

    // Discard the old rating and print the new rating
    fprintf(temp, "%.1f ", rating);
    token = strtok(NULL, " ");

    while (token != NULL)
    {
        fprintf(temp, "%s", token);
        token = strtok(NULL, " ");
        if (token)
            fprintf(temp, " "); // Add space only if there's a next token
    }

    // Copy the rest of the lines until the end of the file
    while ((readingRatings = getline(&ratingsLine, &ratingsLen, ratingFile)) != -1)
    {
        fprintf(temp, "%s", ratingsLine);
    }

    // Free dynamically allocated memory
    free(ratingsLine);

    // Close files
    fclose(ratingFile);
    fclose(temp);

    // Delete old ratings file and rename the temp file to the ratings file name
    remove(USER_RATINGS_FILE);
    rename(TEMP_RATING_FILE, USER_RATINGS_FILE);

    return 1;
}

void rateMovie()
{

    char username[MAX_NAME_LENGTH];

    printf("Enter your username: ");
    scanf("%s", username);
    toLowerCase(username);

    if (!checkUserExists(username))
    {
        printf("User not found. Please register first.\n");
        return;
    }

    int userIndex = getUserIndex(username);

    displayMovies();

    int movieIndex;
    do
    {
        printf("Enter the movie index number to rate (1-10): ");
        scanf("%d", &movieIndex);

        if (movieIndex < 1 || movieIndex > MAX_MOVIES)
        {
            printf("Invalid movie index. Please enter a valid index.\n");
        }
    } while (movieIndex < 1 || movieIndex > MAX_MOVIES);

    float rating;
    do
    {
        printf("Enter your rating (1-5): ");
        scanf("%f", &rating);

        if (rating < 1 || rating > 5)
        {
            printf("Invalid rating. Please enter a rating between 1 and 5.\n");
        }

    } while (rating < 1 || rating > 5);

    updateUserRating(userIndex, movieIndex, rating);

    printf("Rating recorded successfully.\n");
}

int retrieveMovieData(int position, char *name)
{
    FILE *fptr = fopen(MOVIE_DATABASE_FILE, "r");
    if (!fptr)
    {
        printf("Error opening movie database file.\n");
        return 0;
    }

    char line[500];
    int currentLine = 0;
    while (fgets(line, sizeof(line), fptr) != NULL)
    {
        if (currentLine == position)
        {
            line[strcspn(line, "\n")] = 0;

            // Extract movie name from line
            char *token = strtok(line, " ");
            if (token)
            {
                strcpy(name, token);
                // Replace underscores in movie names with spaces
                for (int i = 0; name[i] != '\0'; i++)
                {
                    if (name[i] == '_')
                    {
                        name[i] = ' ';
                    }
                }
            }

            fclose(fptr);
            return 1; // Indicate success
        }
        currentLine++;
    }

    fclose(fptr);
    return 0; // Indicate that the position was not found
}

int getRecommendations(int userId, float scores[])
{
    // Open user ratings file
    FILE *fptr = fopen(USER_RATINGS_FILE, "r");
    if (!fptr)
    {
        printf("Error: Ratings file not found.\n");
        return 0;
    }

    int numUsers, numMovies;
    //first line of the file contains the number of users and movies
    fscanf(fptr, "%d %d\n", &numUsers, &numMovies);

    float cumulativeMovieScores[numMovies];
    int cumulativeMovieRaters[numMovies];
    int userRatedMovies[numMovies];
    
    for (int i = 0; i < numMovies; i++)
    {
        cumulativeMovieScores[i] = 0;
        cumulativeMovieRaters[i] = 0;
        userRatedMovies[i] = 0;
    }

    char line[1024];
    int currentUserIndex = 1;
    while (fgets(line, sizeof(line), fptr) != NULL) {
        char *token = strtok(line, " ");
        for (int i = 0; token && i < numMovies; i++) {
            float rating = atof(token);
            if (currentUserIndex == userId && rating > 0) {
                userRatedMovies[i] = 1; // Mark movie as rated by the user
            }
            if (currentUserIndex != userId && rating > 0) {
                cumulativeMovieScores[i] += rating;
                cumulativeMovieRaters[i]++;
            }
            token = strtok(NULL, " ");
        }
        currentUserIndex++;
    }

    // Calculate average rating for each movie not rated by the user
    for (int i = 0; i < numMovies; i++) {
        if (cumulativeMovieRaters[i] > 0 && userRatedMovies[i] == 0) {
            scores[i] = cumulativeMovieScores[i] / cumulativeMovieRaters[i];
        } else {
            scores[i] = 0; // No ratings or movie has been rated by the user
        }
    }

    fclose(fptr);
    return 1;
}

void recommendMovie()
{
    char *recName = malloc(256 * sizeof(char));
    if (recName == NULL)
    {
        printf("Failed to allocate recName.\n");
        exit(1);
    }

    char username[MAX_NAME_LENGTH];

    printf("Enter your username: ");
    scanf("%s", username);
    toLowerCase(username);

    int userId = getUserIndex(username);

    float rec[MAX_MOVIES];
    if (userId != -1 && getRecommendations(userId, rec))
    {
        printf("\n***** Recommended Movies *****\n");
        int recNumber = 1;
        for (int i = 0; i < num_movies; i++)
        {
            if (rec[i] > 0.0)
            { 
                // Only recommend movies with scores above 0
                retrieveMovieData(i, recName);
                printf("%d. %s - Predicted rating: %1.1f\n", recNumber++, recName, rec[i]);
            }
        }
    }
    free(recName);
}