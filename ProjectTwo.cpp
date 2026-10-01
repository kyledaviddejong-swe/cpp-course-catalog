/*
* Developer: Kyle DeJong
* Date: 4/19/2026
* Project: SNHU CS-300 Module 7 Project 2
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Course objects contain a course number, title, and vector list of preq courses
struct Course {
    std::string courseNumber, title;
    std::vector<std::string> prerequisites;

    Course() { }
    Course(std::string courseNumber, std::string title) {
        this->courseNumber = courseNumber;
        this->title = title;
    }
};

// Node object contains a Course object and pointers to child left/right node in BST pattern
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node() {
        left = right = nullptr;
    }
    Node(Course course) : Node() {
        this->course = course;
    }
};

// Trim surrounding whitespace, including the carriage return in Windows CSV files.
std::string Trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

// Use the same ASCII case normalization for loaded IDs and user searches.
std::string NormalizeCourseNumber(std::string value) {
    value = Trim(value);
    for (char& ch : value) {
        if (ch >= 'a' && ch <= 'z') {
            ch += ('A' - 'a');
        }
    }
    return value;
}

// Split the supported comma-delimited format; quoted fields are not supported.
std::vector<std::string> LineTokenizer(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    // Divide the ss into tokens around the commas and return the storage vector 
    while (std::getline(ss, token, ',')) {
        tokens.push_back(Trim(token));
    }
    return tokens;
}

// Starting from a given root, find and insert a new Node containing a given Course in BST format
Node* InsertCourse(Node* root, const Course& course) {
    // Base case, bottom of the tree
    if (root == nullptr) {
        // Create a new Node on the heap and return the handle
        Node* newNode = new Node(course);
        return newNode;
    }

    // Recursively loop until the proper position (nullptr) is found, then assign the handle to the new Node
    if (course.courseNumber < root->course.courseNumber) {
        root->left = InsertCourse(root->left, course);
    }
    else {
        root->right = InsertCourse(root->right, course);
    }

    // Return updated subtree root
    return root;
}

// Load course data from file and populate a BST, return false on any failures
bool LoadCourses(const std::string& fileName, Node*& root) {
    std::ifstream file(fileName);

    // Auto-append the file type (.csv) to the user input in case they've forgotten
    if (!file.is_open()) {
        file.open(fileName + ".csv");
    }

    // Last chance check that the file opened
    if (!file.is_open()) {
        std::cout << "Failed to open file: \"" << fileName << "\".\n";
        return false;
    }

    std::vector<Course> tempCourses;
    std::vector<std::string> courseNumbers;
    std::string line;

    // Read and tokenize each line 
    while (std::getline(file, line)) {
        if (Trim(line).empty()) {
            continue;
        }
        if (line.find('"') != std::string::npos) {
            std::cout << "Quoted fields are not supported.\n";
            return false;
        }
        std::vector<std::string> tokens = LineTokenizer(line);

        // Every valid course needs a course number and title, otherwise reject the load attempt
        if (tokens.size() < 2 || tokens[0].empty() || tokens[1].empty()) {
            std::cout << "Invalid course formatting.\n";
            file.close();
            return false;
        }

        const std::string courseID = NormalizeCourseNumber(tokens[0]);
        if (std::find(courseNumbers.begin(), courseNumbers.end(), courseID) != courseNumbers.end()) {
            std::cout << "Duplicate course number: " << courseID << "\n";
            return false;
        }

        // Create new Course instance
        Course newCourse(courseID, tokens[1]);

        // Store prerequisites, if any
        for (size_t i = 2; i < tokens.size(); i++) {
            if (!tokens.at(i).empty()) {
                newCourse.prerequisites.push_back(NormalizeCourseNumber(tokens[i]));
            }
        }

        // Save course data for later validation/insertion
        courseNumbers.push_back(newCourse.courseNumber);
        tempCourses.push_back(newCourse);
    }

    if (file.bad()) {
        std::cout << "Failed to read course data.\n";
        return false;
    }
    file.close();

    // Prevent a successful load of an empty file from deleting an existing tree
    if (tempCourses.empty()) {
        std::cout << "No course data found.\n";
        return false;
    }

    // Validate every prerequisite before building the replacement BST
    for (Course& course : tempCourses) {
        for (const std::string& prerequisite : course.prerequisites) {
            bool prerequisiteFound = false;
            // Immediately break the loop if a preq is valid 
            for (const std::string& validCourseNumber : courseNumbers) {
                if (prerequisite == validCourseNumber) {
                    prerequisiteFound = true;
                    break;
                }
            }
            // If a preq is invalid, reject the load attempt
            if (!prerequisiteFound) {
                std::cout << "Invalid prerequisite: " << prerequisite << "\n";
                return false;
            }
        }
    }
    for (const Course& course : tempCourses) {
        root = InsertCourse(root, course);
    }
    // Only if a valid, populated load/BST creation occurred 
    return true;
}

// Search for course by course ID starting at given subtree node
Node* SearchCourse(Node* root, const std::string& courseNumber) {
    if (root == nullptr) {
        return nullptr;
    }

    if (root->course.courseNumber == courseNumber) {
        return root;
    }
    // Recurse left or right as per BST pattern
    if (root->course.courseNumber > courseNumber) {
        return SearchCourse(root->left, courseNumber);
    }
    else {
        return SearchCourse(root->right, courseNumber);
    }
}

// Print a course ID, title, and (optionally) prerequisites
void PrintCourse(Node* root, const Course& course, bool printPreq = true) {
    std::cout << course.courseNumber << ": " << course.title << "\n";

    // Default case, print all preqs
    if (printPreq) {
        if (course.prerequisites.empty()) {
            std::cout << "Prerequisites: None\n";
        }
        else {
            std::cout << "Prerequisites:\n";
            // Scan the BST for the title to match the ID
            for (const std::string& preq : course.prerequisites) {
                Node* preqNode = SearchCourse(root, preq);
                // Output preq ID and title
                if (preqNode != nullptr) {
                    std::cout << preqNode->course.courseNumber << ": "
                        << preqNode->course.title << "\n";
                }             
            }
        }
    }
}

// Print all course info from subtree root and down, in alphanumeric order (low to high) 
void PrintAllCourses(Node* root) {
    if (root != nullptr) {
        PrintAllCourses(root->left);
        // Don't print preqs
        PrintCourse(root, root->course, false);
        PrintAllCourses(root->right);
    }
}

// Recursively destroys the BST starting from param node
void DestroyTree(Node* root) {
    if (root != nullptr) {
        DestroyTree(root->left);
        DestroyTree(root->right);
        delete root;
    }
}

int main() {
    Node* root = nullptr;
    Node* newRoot = nullptr;
    std::string fileName, courseNumber, input;
    int choice = 0;

    // Menu, valid options {1, 2, 3, 9}. 
    while (choice != 9) {
        std::cout << "\nMenu:\n"
            << "1. Load Courses\n"
            << "2. Print Course List\n"
            << "3. Search for a Course\n"
            << "9. Exit\n"
            << "Enter your choice: ";

        if (!std::getline(std::cin, input)) {
            break;
        }
        std::stringstream ss(input);
        int parsedChoice;
        char extra;
        // Reject trailing characters before changing the loop-control value.
        if (!(ss >> parsedChoice) || (ss >> extra)) {
            std::cout << "Invalid option.\n";
            continue;
        }
        choice = parsedChoice;
        // Menu choices: 1 - Create BST from file, 2 - Print all course data, 3 - Search for a specific course + preqs, 9 - Exit
        switch (choice) {    
        // Create BST from file
        case 1:
            std::cout << "Enter file name: ";
            if (!std::getline(std::cin, fileName)) {
                choice = 9;
                break;
            }
            fileName = Trim(fileName);

            // If a load operation occurs, attempt to create the new BST, and if successful, delete the old one if it exists
            newRoot = nullptr;
            if (LoadCourses(fileName, newRoot)) {
                DestroyTree(root);
                root = newRoot;
            }
            // Otherwise destroy the unsuccessful new BST
            else {
                DestroyTree(newRoot);
                if (root != nullptr) {
                    std::cout << "Course load failed. Any existing data kept.\n";
                }
            }
            break;
        // Print all course data
        case 2:
            if (root == nullptr) {
                std::cout << "No courses loaded.\n";
            }
            else {
                PrintAllCourses(root);
            }
            break;
        // 3 - Search for a specific course + preqs
        case 3:
            if (root == nullptr) {
                std::cout << "No courses loaded.\n";
            }
            else {
                std::cout << "Enter a course number: ";
                if (!std::getline(std::cin, courseNumber)) {
                    choice = 9;
                    break;
                }
                courseNumber = NormalizeCourseNumber(courseNumber);

                Node* foundCourse = SearchCourse(root, courseNumber);

                if (foundCourse == nullptr) {
                    std::cout << "Course not found.\n";
                }
                else {
                    PrintCourse(root, foundCourse->course);
                }
            }
            break;
        // Exit
        case 9:
            break;
        // All other inputs 
        default:
            std::cout << "Invalid option.\n";
            break;
        }
    }

    // Clean up and exit
    DestroyTree(root);
    root = nullptr;
    std::cout << "Exiting program.\n";
    return 0;
}
