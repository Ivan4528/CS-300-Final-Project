// ProjectTwo.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>

struct Course {
    std::string number;                // e.g., "CSCI200"
    std::string title;                 // e.g., "Data Structures"
    std::vector<std::string> prereqs;  // e.g., {"CSCI101", "MATH201"}
};

// ---------- Utility helpers ----------

static inline std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

static inline std::string toUpper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Split by ',' and keep empty fields
static std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> out;
    std::string token;
    std::stringstream ss(line);
    while (std::getline(ss, token, ',')) {
        out.push_back(token);
    }
    // If line ended with a comma, std::getline won't produce a trailing empty token.
    // But our data may have extra commas;
    return out;
}

// ---------- Advising Program (Hash Table) ----------

class AdvisingProgram {
public:
    // Load from file; returns true on success
    bool loadFromFile(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) {
            std::cerr << "Error: could not open file \"" << filename << "\".\n";
            return false;
        }

        // Clear previous data for reload
        courses_.clear();

        // First pass: read lines, parse number/title/prereqs, store courses by number
        std::string line;
        size_t lineNo = 0;
        while (std::getline(fin, line)) {
            ++lineNo;
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;

            auto tokens = splitCSV(trimmed);
            if (tokens.size() < 2) {
                std::cerr << "Format warning (line " << lineNo << "): fewer than 2 fields.\n";
                continue;
            }

            std::string number = toUpper(trim(tokens[0]));
            std::string title  = trim(tokens[1]);
            if (number.empty() || title.empty()) {
                std::cerr << "Format warning (line " << lineNo << "): empty course number or title.\n";
                continue;
            }

            Course c;
            c.number = number;
            c.title = title;

            // Remaining tokens are prerequisites (trim, normalize, ignore empties)
            if (tokens.size() > 2) {
                for (size_t i = 2; i < tokens.size(); ++i) {
                    std::string p = toUpper(trim(tokens[i]));
                    if (!p.empty()) c.prereqs.push_back(p);
                }
            }

            // Insert or update
            courses_[c.number] = std::move(c);
        }
        fin.close();

        if (courses_.empty()) {
            std::cerr << "Warning: no valid course records were loaded.\n";
            return false;
        }

        // Second pass: validate that each prerequisite exists in the map
        size_t missingCount = 0;
        for (auto const& kv : courses_) {
            auto const& course = kv.second;
            for (auto const& p : course.prereqs) {
                if (courses_.find(p) == courses_.end()) {
                    ++missingCount;
                    std::cerr << "Validation warning: prerequisite \"" << p
                              << "\" not found for course " << course.number << ".\n";
                }
            }
        }

        std::cout << "Loaded " << courses_.size() << " courses";
        if (missingCount > 0) {
            std::cout << " with " << missingCount << " missing prerequisite reference(s)";
        }
        std::cout << ".\n";
        loaded_ = true;
        return true;
    }

    // Print all courses sorted alphanumerically by course number
    void printCourseList() const {
        if (!loadedCheck()) return;
        std::vector<std::string> keys;
        keys.reserve(courses_.size());
        for (auto const& kv : courses_) keys.push_back(kv.first);

        std::sort(keys.begin(), keys.end()); // lexicographic sort is fine for course codes

        std::cout << "Here is a sample schedule:\n";
        for (auto const& code : keys) {
            auto const& c = courses_.at(code);
            std::cout << c.number << ", " << c.title << "\n";
        }
    }

    // Print a single course's details (title and prerequisites with titles)
    void printCourseDetails(const std::string& userInput) const {
        if (!loadedCheck()) return;
        std::string code = toUpper(trim(userInput));
        if (code.empty()) {
            std::cout << "Please enter a course number.\n";
            return;
        }

        auto it = courses_.find(code);
        if (it == courses_.end()) {
            std::cout << "Course " << code << " not found.\n";
            return;
        }

        auto const& c = it->second;
        std::cout << c.number << ", " << c.title << "\n";

        if (c.prereqs.empty()) {
            std::cout << "Prerequisites: None\n";
        } else {
            std::cout << "Prerequisites: ";
            for (size_t i = 0; i < c.prereqs.size(); ++i) {
                const std::string& p = c.prereqs[i];
                auto pit = courses_.find(p);
                if (pit != courses_.end()) {
                    std::cout << pit->second.number; // number
                    if (i + 1 < c.prereqs.size()) std::cout << ", ";
                } else {
                    // Show the code even if the title is missing
                    std::cout << p;
                    if (i + 1 < c.prereqs.size()) std::cout << ", ";
                }
            }
            std::cout << "\n";

            // Optionally also show titles for prerequisites:
            std::cout << "Prerequisite titles: ";
            for (size_t i = 0; i < c.prereqs.size(); ++i) {
                const std::string& p = c.prereqs[i];
                auto pit = courses_.find(p);
                if (pit != courses_.end()) {
                    std::cout << pit->second.title;
                } else {
                    std::cout << "(missing: " << p << ")";
                }
                if (i + 1 < c.prereqs.size()) std::cout << ", ";
            }
            std::cout << "\n";
        }
    }

    bool isLoaded() const { return loaded_; }

private:
    bool loadedCheck() const {
        if (!loaded_) {
            std::cout << "Please load data first (Option 1).\n";
            return false;
        }
        return true;
    }

    std::unordered_map<std::string, Course> courses_;
    bool loaded_ = false;
};

// ---------- Main Menu ----------

static void printMenu() {
    std::cout << "Welcome to the course planner.\n";
    std::cout << "1. Load Data Structure.\n";
    std::cout << "2. Print Course List.\n";
    std::cout << "3. Print Course.\n";
    std::cout << "9. Exit\n";
    std::cout << "What would you like to do? ";
}

int main() {
    AdvisingProgram app;

    while (true) {
        printMenu();
        std::string choiceLine;
        if (!std::getline(std::cin, choiceLine)) break;
        choiceLine = trim(choiceLine);
        if (choiceLine.empty()) continue;

        int choice = -1;
        try {
            choice = std::stoi(choiceLine);
        } catch (...) {
            // Non-numeric input
            std::cout << choiceLine << " is not a valid option.\n";
            continue;
        }

        if (choice == 1) {
            std::cout << "Enter the file name to load (e.g., courses.txt): ";
            std::string fname;
            std::getline(std::cin, fname);
            fname = trim(fname);
            if (fname.empty()) {
                std::cout << "No file name entered.\n";
                continue;
            }
            app.loadFromFile(fname);
        } else if (choice == 2) {
            if (!app.isLoaded()) {
                std::cout << "Please load data first (Option 1).\n";
            } else {
                app.printCourseList();
            }
        } else if (choice == 3) {
            if (!app.isLoaded()) {
                std::cout << "Please load data first (Option 1).\n";
            } else {
                std::cout << "What course do you want to know about? ";
                std::string code;
                std::getline(std::cin, code);
                app.printCourseDetails(code);
            }
        } else if (choice == 9) {
            std::cout << "Thank you for using the course planner!\n";
            break;
        } else {
            std::cout << choice << " is not a valid option.\n";
        }
    }

    return 0;
}
