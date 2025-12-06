#ifndef SOLUTION_DOCUMENT_H
#define SOLUTION_DOCUMENT_H

#include "Solution.h"
#include <string>
#include <memory>
#include <vector>

class SolutionDocument {
public:
    SolutionDocument();
    virtual ~SolutionDocument() = default;
    
    // Document identification
    void setName(const std::string& name) { name_ = name; }
    std::string getName() const { return name_; }
    
    void setPath(const std::string& path) { path_ = path; }
    std::string getPath() const { return path_; }
    
    bool isModified() const { return modified_; }
    void setModified(bool modified) { modified_ = modified; }
    
    // Pure virtual methods - must be implemented by derived classes
    virtual bool load(const std::string& file_path) = 0;
    virtual bool save(const std::string& file_path = "") = 0;
    virtual bool saveAs(const std::string& file_path) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    
    // Document content management
    virtual void clear() = 0;
    virtual bool isEmpty() const = 0;
    
    // Document operations
    virtual std::unique_ptr<SolutionDocument> copy() const = 0;
    virtual void useFor(const std::string& purpose) = 0;
    virtual bool sendTo(const std::string& destination, const std::string& format = "") = 0;
    
    // Import/Export operations
    virtual bool import(const std::string& file_path, const std::string& format = "") = 0;
    virtual bool exportTo(const std::string& file_path, const std::string& format = "") = 0;
    virtual std::vector<std::string> getSupportedImportFormats() const = 0;
    virtual std::vector<std::string> getSupportedExportFormats() const = 0;
    
    // Solution management
    void addSolution(std::unique_ptr<Solution> solution);
    Solution* getSolution(size_t index) const;
    Solution* getSolution(const std::string& name) const;
    size_t getSolutionCount() const { return solutions_.size(); }
    void removeSolution(size_t index);
    void removeSolution(const std::string& name);
    void clearSolutions();
    
    std::vector<Solution*> getAllSolutions() const;
    
    // Document metadata
    void setAuthor(const std::string& author) { author_ = author; }
    std::string getAuthor() const { return author_; }
    
    void setDescription(const std::string& description) { description_ = description; }
    std::string getDescription() const { return description_; }
    
    void setVersion(const std::string& version) { version_ = version; }
    std::string getVersion() const { return version_; }
    
protected:
    std::string name_;
    std::string path_;
    bool modified_;
    std::string author_;
    std::string description_;
    std::string version_;
    std::vector<std::unique_ptr<Solution>> solutions_;
};

#endif // SOLUTION_DOCUMENT_H

