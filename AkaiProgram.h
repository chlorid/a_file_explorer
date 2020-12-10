
#ifndef AKAI_PROGRAM_H_
#define AKAI_PROGRAM_H_
#include <Wt/WTableView.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>


#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStringListModel.h>
// #include <Wt/WTableView.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WComboBox.h>
#include <Wt/WAny.h>

extern "C" {
#include "akaiutil/akaiutil_file.h"	
#include "akaiutil/akaiutil.h"
}



using namespace Wt;

class AkaiKeyGroupEditor : WTableView {

private:
	struct file_s programFile; 
	unsigned char *filedata;
	std::unique_ptr<WContainerWidget> rootContainer;
	WContainerWidget *rootCnt_;
	struct akai_program1000_s *programHeader;
	struct akai_program1000kgvelzone_s *keyGroupVelocityHeader;
	struct akai_program1000kg_s *keyGroupHeader;
	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, std::string itemValue);
	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, unsigned char *itemValue);
	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, unsigned char itemValue);
	
	
	
// 	auto table = Wt::cpp14::make_unique<Wt::WTableView>();
	
public:
	int loadProgramFile(std::string filePath, std::string fileName);
	void fillEditWindow();
	std::unique_ptr<WContainerWidget> getEditWindow();
// 	WContainerWidget *getEditWindow();
	AkaiKeyGroupEditor();// : WTableView();
// 	~AkaiKeyGroupEditor();
// 	~AkaiKeyGroupEditor();
	
	
};


#endif
