#include "verifyset.h"
#include <string.h>
#include <QDebug>
#include <QFileInfo>


/*!
 * \fn VerifySet::VerifySet
 * \brief Takes absolute path of project XML file
 * \param projectXMLPath
 * \param projectXML_formatPath
 */
VerifySet::VerifySet(QString projectXMLPath, QString projectXML_formatPath)
{
    this->ProjectXMLPath = projectXMLPath.toStdString();
    this->ProjectXML_formatPath = projectXML_formatPath.toStdString();
}

/*!
 * \fn VerifySet::testProjectXML
 * \brief Compares the user's project XML file and the current format of that file.
 * \return Returns 0 if not error found else -1 is returned
 * \note Uses pugi xml parsing library
 */
int VerifySet::testProjectXML()
{
    const char *cstr = ProjectXMLPath.c_str();
    pugi::xml_parse_result result = doc.load_file(cstr);
    if (!result) {
        error_code = error_codes::project_file_opening_error;
        error_string = "Project XML not loaded properly";
        return -1;
    }
    if (!LoadProjectXMLFormat()) {
        qDebug() << "Error in loading docFormat";
        return -1;
    }

    // Traverse both XML file and check parallely for errors
    pugi::xml_node nodeTest = doc.child("Project"); // Node to be tested
    pugi::xml_node nodeFormat = docFormat.child("Project"); // Node against which above node is to be tested

    if (!verifyNode(nodeTest, nodeFormat)) {
        qDebug() << "Some Error occurred\n";
        return -1;
    }
    return 0;
}

/*!
 * \fn VerifySet::LoadProjectXMLFormat
 * \brief Loads the format of XML in docFormat var
 * \return Returns true if no error found else false is returned
 */
bool VerifySet::LoadProjectXMLFormat()
{
    QFileInfo xmlFormatFile(QString::fromStdString(ProjectXML_formatPath));
    pugi::xml_parse_result result = docFormat.load_file(xmlFormatFile.absoluteFilePath().toStdString().c_str());
    if (!result)
        return false;
    return true;
}

/*!
 * \fn VerifySet::getErrorCode
 * \brief Returns the error code
 * \return int error_code
 */
int VerifySet::getErrorCode()
{
    return error_code;
}

/*!
 * \fn VerifySet::getErrorString
 * \brief Returns the error string
 * \return error_string
 */
QString VerifySet::getErrorString()
{
    return error_string;
}

/*!
 * \fn VerifySet::verifyNode
 * \param nodeTest
 * \param nodeFormat
 * \brief Compares the user's XML node(tag) against the format of node in the current format of project XML
 * \return Returns true if verification completes else returns false
 * \note XML in pugi library is stored as a tree ( https://pugixml.org/docs/manual.html#dom )
 * \details Traversing tree in the preOrder fashion
 */
bool VerifySet::verifyNode(pugi::xml_node nodeTest, pugi::xml_node nodeFormat)
{
    // Check if one of the node is NULL and other is not
    if ((nodeTest == NULL && nodeFormat != NULL) || (nodeTest != NULL && nodeFormat == NULL)) {
        error_code = error_codes::node_availability_error;
        error_string = "Tag is missing or some extra node is present";
        return false;
    }
    // Check if both the nodes are NULL
    else if (nodeTest == NULL && nodeFormat == NULL) {
        return true;
    }

    // Check node name
    if (strcmp(nodeTest.name(), nodeFormat.name()) != 0)
    {
        error_code = error_codes::node_name_error;
        error_string = "Tag name does not matches with that of in format of project XML";
        return false;
    }

    // Check attributes' names
    if (!verifyNodeAttributes(nodeTest, nodeFormat)) {
        error_code = error_codes::attribute_error;
        error_string = "Some issue with attribute(s) in a tag";
        return false;
    }

    // Check child node(s)
    pugi::xml_node childTest = nodeTest.first_child();
    pugi::xml_node childFormat = nodeFormat.first_child();
    if (!verifyNode(childTest, childFormat)) {
        return false;
    }

    // Check sibling node(s)
    pugi::xml_node siblingTest = nodeTest.next_sibling();
    pugi::xml_node siblingFormat = nodeFormat.next_sibling();
    if (!verifyNode(siblingTest, siblingFormat)) {
        return false;
    }
    return true;
}

/*!
 * \fn VerifySet::verifyNodeAttributes
 * \brief Compares one node attribute name with the another
 * \param nodeTest
 * \param nodeFormat
 * \return Returns true if verification completes else returns false
 */
bool VerifySet::verifyNodeAttributes(pugi::xml_node nodeTest, pugi::xml_node nodeFormat)
{
    pugi::xml_attribute attrTest = nodeTest.first_attribute();
    pugi::xml_attribute attrFormat = nodeFormat.first_attribute();
    if (attrTest == NULL && attrFormat == NULL)
        return true;
    else if ((attrTest == NULL && attrFormat != NULL) || (attrTest != NULL && attrFormat == NULL))
        return false;

    while (attrTest && attrFormat)
    {
        if (strcmp(attrTest.name(), attrFormat.name()) != 0) {
            return false;
        }
        attrTest = attrTest.next_attribute();
        attrFormat = attrFormat.next_attribute();
    }
    if (attrTest != attrFormat)
        return false;
    return true;
}
