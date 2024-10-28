#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define constants for maximum number of boats and name length
#define MAX_BOATS 120
#define MAX_NAME_LENGTH 127
#define MAX_LENGTH 100

// Enum to represent different types of boat placements
typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

// Union to store extra information based on the placement type
typedef union {
    int slipNumber;
    char bayLetter;
    char trailorLicense[10];
    int storageNumber;
} ExtraInfo;

// Struct to represent a boat and its attributes
typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int length;
    PlaceType place;
    ExtraInfo extra;
    double amountOwed;
} Boat;

// Global array to store pointers to Boat structs
Boat* boats[MAX_BOATS];
int boatCount = 0;

// Function prototypes
void loadBoats(const char* filename);
void saveBoats(const char* filename);
void printInventory();
void addBoat(const char* boatData);
void removeBoat(const char* boatName);
void makePayment(const char* boatName);
void updateMonthlyCharges();
int compareBoats(const void* a, const void* b);
PlaceType stringToPlaceType(const char* placeString);
const char* placeTypeToString(PlaceType place);

int main(int argc, char* argv[]) {
    // Check if the correct number of command-line arguments is provided
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Load boat data from the specified file
    loadBoats(argv[1]);

    char choice;
    char input[256];

    // Display welcome message
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n\n");

    // Main program loop
    do {
        // Display menu and get user choice
        printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &choice);
        choice = tolower(choice);

        // Process user choice
        switch (choice) {
            case 'i':
                printInventory();
                break;
            case 'a':
                printf("Please enter the boat data in CSV format                 : ");
                scanf(" %[^\n]", input);
                addBoat(input);
                printf("\n");
                break;
            case 'r':
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", input);
                removeBoat(input);
                break;
            case 'p':
                {
                    char boatName[MAX_NAME_LENGTH + 1];
                    printf("Please enter the boat name                               : ");
                    scanf(" %[^\n]", boatName);
                    makePayment(boatName);
                }
                break;
            case 'm':
                updateMonthlyCharges();
                printf("\n");
                break;
            case 'x':
                printf("\nExiting the Boat Management System\n");
                break;
            default:
                printf("Invalid option %c\n\n", toupper(choice));
        }
    } while (choice != 'x');

    // Save boat data back to the file
    saveBoats(argv[1]);

    // Free allocated memory for boats
    for (int i = 0; i < boatCount; i++) {
        free(boats[i]);
    }

    return 0;
}

// Load boats from CSV file
void loadBoats(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file) && boatCount < MAX_BOATS) {
        // Allocate memory for a new boat
        Boat* newBoat = malloc(sizeof(Boat));
        if (!newBoat) {
            perror("Memory allocation failed");
            exit(1);
        }

        // Parse the CSV line and populate the boat struct
        char placeStr[10];
        char extraStr[20];
        sscanf(line, "%[^,],%d,%[^,],%[^,],%lf",
               newBoat->name,
               &newBoat->length,
               placeStr,
               extraStr,
               &newBoat->amountOwed);

        newBoat->place = stringToPlaceType(placeStr);

        // Handle extra information based on place type
        switch (newBoat->place) {
            case slip:
                sscanf(extraStr, "%d", &newBoat->extra.slipNumber);
                break;
            case land:
                newBoat->extra.bayLetter = extraStr[0];
                break;
            case trailor:
                strncpy(newBoat->extra.trailorLicense, extraStr, sizeof(newBoat->extra.trailorLicense) - 1);
                newBoat->extra.trailorLicense[sizeof(newBoat->extra.trailorLicense) - 1] = '\0';
                break;
            case storage:
                sscanf(extraStr, "%d", &newBoat->extra.storageNumber);
                break;
            case no_place:
            default:
                // Handle no_place or invalid place type
                break;
        }

        boats[boatCount++] = newBoat;
    }

    fclose(file);
    // Sort boats alphabetically by name
    qsort(boats, boatCount, sizeof(Boat*), compareBoats);
}

// Save boats to CSV file
void saveBoats(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        exit(1);
    }

    // Write each boat's data to the file
    for (int i = 0; i < boatCount; i++) {
        fprintf(file, "%s,%d,%s,", boats[i]->name, boats[i]->length, placeTypeToString(boats[i]->place));
        switch (boats[i]->place) {
            case slip:
                fprintf(file, "%d", boats[i]->extra.slipNumber);
                break;
            case land:
                fprintf(file, "%c", boats[i]->extra.bayLetter);
                break;
            case trailor:
                fprintf(file, "%s", boats[i]->extra.trailorLicense);
                break;
            case storage:
                fprintf(file, "%d", boats[i]->extra.storageNumber);
                break;
            case no_place:
            default:
                fprintf(file, "N/A");
                break;
        }
        fprintf(file, ",%.2f\n", boats[i]->amountOwed);
    }

    fclose(file);
}

// Print boat inventory
void printInventory() {
    for (int i = 0; i < boatCount; i++) {
        printf("%-20s %3d' ", boats[i]->name, boats[i]->length);
        switch (boats[i]->place) {
            case slip:
                printf("   slip   #%3d", boats[i]->extra.slipNumber);
                break;
            case land:
                printf("   land      %c", boats[i]->extra.bayLetter);
                break;
            case trailor:
                printf("trailor %6s", boats[i]->extra.trailorLicense);
                break;
            case storage:
                printf("storage   #%3d", boats[i]->extra.storageNumber);
                break;
            case no_place:
            default:
                printf("   N/A        ");
                break;
        }
        printf("   Owes $%7.2f\n", boats[i]->amountOwed);
    }
    printf("\n");
}

// Add a new boat
void addBoat(const char* boatData) {
    if (boatCount >= MAX_BOATS) {
        printf("Maximum number of boats reached.\n");
        return;
    }

    // Allocate memory for a new boat
    Boat* newBoat = malloc(sizeof(Boat));
    if (!newBoat) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Parse the input boat data
    char placeStr[10];
    char extraStr[20];
    sscanf(boatData, "%[^,],%d,%[^,],%[^,],%lf",
           newBoat->name,
           &newBoat->length,
           placeStr,
           extraStr,
           &newBoat->amountOwed);

    newBoat->place = stringToPlaceType(placeStr);

    // Handle extra information based on place type
    switch (newBoat->place) {
        case slip:
            sscanf(extraStr, "%d", &newBoat->extra.slipNumber);
            break;
        case land:
            newBoat->extra.bayLetter = extraStr[0];
            break;
        case trailor:
            strncpy(newBoat->extra.trailorLicense, extraStr, sizeof(newBoat->extra.trailorLicense) - 1);
            newBoat->extra.trailorLicense[sizeof(newBoat->extra.trailorLicense) - 1] = '\0';
            break;
        case storage:
            sscanf(extraStr, "%d", &newBoat->extra.storageNumber);
            break;
        case no_place:
        default:
            // Handle no_place or invalid place type
            break;
    }

    // Insert the new boat in the correct position to maintain sorting
    int i;
    for (i = boatCount - 1; i >= 0 && strcmp(boats[i]->name, newBoat->name) > 0; i--) {
        boats[i + 1] = boats[i];
    }
    boats[i + 1] = newBoat;
    boatCount++;
}

// Remove a boat by name
void removeBoat(const char* boatName) {
    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, boatName) == 0) {
            free(boats[i]);
            // Shift remaining boats to fill the gap
            for (int j = i; j < boatCount - 1; j++) {
                boats[j] = boats[j + 1];
            }
            boatCount--;
            
            return;
        }
    }
    printf("No boat with that name\n\n");
}

// Make a payment for a boat
void makePayment(const char* boatName) {
    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, boatName) == 0) {
            double amount;
            printf("Please enter the amount to be paid                       : ");
            scanf("%lf", &amount);
            if (amount > boats[i]->amountOwed) {
                printf("That is more than the amount owed, $%.2f\n\n", boats[i]->amountOwed);
            } else {
                boats[i]->amountOwed -= amount;
            }
            return;
        }
    }
    printf("No boat with that name\n\n");
}

// Update monthly charges for all boats
void updateMonthlyCharges() {
    for (int i = 0; i < boatCount; i++) {
        switch (boats[i]->place) {
            case slip:
                boats[i]->amountOwed += 12.50 * boats[i]->length;
                break;
            case land:
                boats[i]->amountOwed += 14.00 * boats[i]->length;
                break;
            case trailor:
                boats[i]->amountOwed += 25.00 * boats[i]->length;
                break;
            case storage:
                boats[i]->amountOwed += 11.20 * boats[i]->length;
                break;
            case no_place:
            default:
                // No charge for no_place or invalid place type
                break;
        }
    }
}

// Comparison function for qsort to sort boats alphabetically
int compareBoats(const void* a, const void* b) {
    return strcasecmp((*(Boat**)a)->name, (*(Boat**)b)->name);
}

// Convert string to PlaceType enum
PlaceType stringToPlaceType(const char* placeString) {
    if (strcasecmp(placeString, "slip") == 0) return slip;
    if (strcasecmp(placeString, "land") == 0) return land;
    if (strcasecmp(placeString, "trailor") == 0) return trailor;
    if (strcasecmp(placeString, "storage") == 0) return storage;
    return no_place;
}

// Convert PlaceType enum to string
const char* placeTypeToString(PlaceType place) {
    switch (place) {
        case slip: return "slip";
        case land: return "land";
        case trailor: return "trailor";
        case storage: return "storage";
        case no_place:
        default: return "no_place";
    }
}
