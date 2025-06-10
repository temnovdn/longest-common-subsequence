#include "mainwindow.h"           
#include "./ui_mainwindow.h"      
#include <QVBoxLayout>           
#include <QHBoxLayout>            
#include <QFileDialog>           
#include <QTextStream>           
#include <QMessageBox>            
#include <QTimer>                 
#include <QLabel>                 
#include <QSlider>
#include <QGroupBox>

#include "lcs.h"                 

// Constructor for MainWindow, initializes UI and widgets
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) { 
    ui->setupUi(this); // Setup the UI components
    setWindowTitle("Longest Common Subsequence Visualizer");

    QWidget *centralWidget = new QWidget(this); // Create central widget container
    QVBoxLayout *layout = new QVBoxLayout(); // Create vertical layout for central widget

    // Create input group
    QGroupBox *inputGroup = new QGroupBox("Input Sequences");
    QVBoxLayout *inputLayout = new QVBoxLayout();
    
    inputX = new QLineEdit(); // Create input field for sequence X
    inputY = new QLineEdit(); // Create input field for sequence Y
    inputX->setPlaceholderText("Enter sequence X"); // Set placeholder text for inputX
    inputY->setPlaceholderText("Enter sequence Y"); // Set placeholder text for inputY
    
    sequenceXLabel = new QLabel("Sequence X: ");
    sequenceYLabel = new QLabel("Sequence Y: ");
    
    inputLayout->addWidget(sequenceXLabel);
    inputLayout->addWidget(inputX);
    inputLayout->addWidget(sequenceYLabel);
    inputLayout->addWidget(inputY);
    inputGroup->setLayout(inputLayout);
    layout->addWidget(inputGroup);

    // Create control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    computeButton = new QPushButton("Compute LCS"); // Create button to trigger LCS computation
    loadButton = new QPushButton("Load from File"); // Create button to load sequences from file
    saveButton = new QPushButton("Save to File"); // Create button to save sequences to file
    resetButton = new QPushButton("Reset"); // Create button to reset UI and data
    
    buttonLayout->addWidget(loadButton); // Add load button to button layout
    buttonLayout->addWidget(saveButton);               
    buttonLayout->addWidget(computeButton);            
    buttonLayout->addWidget(resetButton);             
    layout->addLayout(buttonLayout);                    

    // Create animation speed control
    QGroupBox *speedGroup = new QGroupBox("Animation Speed");
    QHBoxLayout *speedLayout = new QHBoxLayout();
    animationSpeedSlider = new QSlider(Qt::Horizontal);
    animationSpeedSlider->setMinimum(10);
    animationSpeedSlider->setMaximum(500);
    animationSpeedSlider->setValue(50);
    animationSpeedSlider->setTickPosition(QSlider::TicksBelow);
    animationSpeedSlider->setTickInterval(50);
    
    speedLabel = new QLabel("Speed: 50ms");
    speedLayout->addWidget(animationSpeedSlider);
    speedLayout->addWidget(speedLabel);
    speedGroup->setLayout(speedLayout);
    layout->addWidget(speedGroup);

    // Create visualization area
    QGroupBox *visualizationGroup = new QGroupBox("Dynamic Programming Visualization");
    QVBoxLayout *visualizationLayout = new QVBoxLayout();
    dpTable = new QTableWidget(); // Create table widget to display DP matrix
    visualizationLayout->addWidget(dpTable);
    visualizationGroup->setLayout(visualizationLayout);
    layout->addWidget(visualizationGroup);

    // Create result area
    QGroupBox *resultGroup = new QGroupBox("Result");
    QVBoxLayout *resultLayout = new QVBoxLayout();
    lcsLabel = new QLabel("LCS will appear here");     
    resultLayout->addWidget(lcsLabel);                        
    resultGroup->setLayout(resultLayout);
    layout->addWidget(resultGroup);

    centralWidget->setLayout(layout);                   
    setCentralWidget(centralWidget);               

    // Connect button signals to corresponding slot functions
    connect(computeButton, &QPushButton::clicked, this, &MainWindow::onComputeLCS);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadFromFile);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveToFile);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(animationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::updateAnimationSpeed);
    connect(inputX, &QLineEdit::textChanged, this, &MainWindow::updateSequenceLabels);
    connect(inputY, &QLineEdit::textChanged, this, &MainWindow::updateSequenceLabels);
}

// Destructor, clean up UI object
MainWindow::~MainWindow() {
    delete ui;  // Delete UI instance to free memory
}

// Slot to compute the Longest Common Subsequence when button clicked
void MainWindow::onComputeLCS() {
    QString qx = inputX->text(); // Get text from inputX field
    QString qy = inputY->text(); // Get text from inputY field

    std::string X = qx.toStdString(); // Convert QString to std::string for processing
    std::string Y = qy.toStdString();

    LCS solver(X, Y); // Create LCS solver instance with input strings
    solver.compute(); // Compute the LCS using DP algorithm

    auto dp = solver.getDPTable(); // Retrieve DP matrix after computation
    auto dir = solver.getDirectionTable(); // Retrieve direction matrix for backtracking

    visualizeDP(dp, dir); // Visualize DP and direction tables on UI

    QString result = QString::fromStdString(solver.getLCS());  // Get LCS result as QString
    lcsLabel->setText("Longest Common Subsequence: " + result); // Display LCS result
}

// Function to visualize DP and direction tables on QTableWidget
void MainWindow::visualizeDP(const std::vector<std::vector<int>> &dp, const std::vector<std::vector<char>> &dir) {

    dpMatrix = dp; // Store DP matrix for animation steps
    directionMatrix = dir; // Store direction matrix for animation steps
    currentRow = 0; // Reset current row index for animation
    currentCol = 0; // Reset current column index for animation

    int rows = dp.size(); // Number of rows in DP matrix
    int cols = dp[0].size(); // Number of columns in DP matrix

    dpTable->setRowCount(rows); // Set row count of table widget
    dpTable->setColumnCount(cols);// Set column count of table widget

    // Get input strings from UI for header labeling
    QString qx = inputX->text();  // Sequence X (horizontal)
    QString qy = inputY->text();  // Sequence Y (vertical)

    // Prepare horizontal header labels
    QStringList horizontalLabels;
    horizontalLabels << ""; // Top-left corner cell left blank

    if (qx.length() < qy.length()) {  // If X shorter than Y, shift columns by adding empty label
        horizontalLabels << "";   // Add extra empty label to shift labels right
        for (QChar ch : qx)       // Add each character of X as header label
            horizontalLabels << QString(ch);

        while (horizontalLabels.size() < cols)  // Pad remaining columns with empty strings
            horizontalLabels << "";
    } else {
        for (QChar ch : qx) // Otherwise, normal labeling with X characters
            horizontalLabels << QString(ch);

        while (horizontalLabels.size() < cols) // Pad remaining columns if any
            horizontalLabels << "";
    }
    dpTable->setHorizontalHeaderLabels(horizontalLabels); // Set horizontal headers on table

    // Prepare vertical header labels
    QStringList verticalLabels;
    verticalLabels << ""; // Top-left corner cell blank

    for (QChar ch : qy) // Add each character of Y as header label
        verticalLabels << QString(ch);

    while (verticalLabels.size() < rows) // Pad remaining rows if any
        verticalLabels << "";

    dpTable->setVerticalHeaderLabels(verticalLabels); // Set vertical headers on table

    // Clear all cells in the table by setting empty items
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            dpTable->setItem(i, j, new QTableWidgetItem(""));

    // Start animation by scheduling the first step with delay of 50ms
    QTimer::singleShot(50, this, SLOT(animateStep()));
}

// Slot to load sequences from a file
void MainWindow::onLoadFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Input File", "", "Text Files (*.txt)"); // Open file dialog
    if (fileName.isEmpty()) return; // Return if no file selected

    QFile file(fileName); // Create file object
    if (file.open(QIODevice::ReadOnly)) {     
        QTextStream in(&file); 
        inputX->setText(in.readLine()); 
        inputY->setText(in.readLine());        
        file.close();                           
    }
}

// Slot to save current sequences to a file
void MainWindow::onSaveToFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Sequences", "", "Text Files (*.txt)"); // Save file dialog
    if (fileName.isEmpty()) return;            

    QFile file(fileName); // Create file object
    if (file.open(QIODevice::WriteOnly)) {  
        QTextStream out(&file); // Create text stream for file
        out << inputX->text() << "\n" << inputY->text(); // Write both sequences to file separated by newline
        file.close();                           // Close the file
    }
}

// Slot to animate one step of DP visualization
void MainWindow::animateStep() {
    if (currentRow >= dpMatrix.size()) // If all rows processed, stop animation
        return;

    if (currentCol >= dpMatrix[0].size()) { 
        currentCol = 0; // Reset column to 0
        ++currentRow; // Increment row
    }

    if (currentRow < dpMatrix.size()) { // If still rows left to process
        int val = dpMatrix[currentRow][currentCol]; // Get DP value at current cell
        char dir = directionMatrix[currentRow][currentCol];// Get direction character

        QString arrow; // Variable to store arrow symbol
        switch (dir) { 
            case 'D': arrow = "↖"; break; // Diagonal (match)
            case 'U': arrow = "↑"; break; // Up
            case 'L': arrow = "←"; break; // Left
        }

        QString text = QString::number(val);  // Convert DP value to string
        if (currentRow > 0 && currentCol > 0) // Add arrow if not in first row/column
            text += " " + arrow;

        auto *item = dpTable->item(currentRow, currentCol); // Get table item for current cell
        item->setText(text); // Set text to show DP value and arrow
        item->setForeground(Qt::black); // Set text color to black

        highlightCurrentCell(currentRow, currentCol);

        ++currentCol; // Move to next column
        QTimer::singleShot(animationDelay, this, SLOT(animateStep()));  // Schedule next animation step after 50ms
    }
}

void MainWindow::updateAnimationSpeed(int value) {
    animationDelay = value;
    speedLabel->setText(QString("Speed: %1ms").arg(value));
}

void MainWindow::highlightCurrentCell(int row, int col) {
    // Reset all cells to white
    for (int i = 0; i < dpTable->rowCount(); ++i) {
        for (int j = 0; j < dpTable->columnCount(); ++j) {
            if (auto item = dpTable->item(i, j)) {
                item->setBackground(Qt::white);
            }
        }
    }
    
    // Highlight current cell
    if (auto item = dpTable->item(row, col)) {
        item->setBackground(Qt::yellow);
    }
}

void MainWindow::updateSequenceLabels() {
    QString x = inputX->text();
    QString y = inputY->text();
    sequenceXLabel->setText(QString("Sequence X: %1").arg(x));
    sequenceYLabel->setText(QString("Sequence Y: %1").arg(y));
}

// Slot to reset UI and internal data to initial state
void MainWindow::onReset() {
    inputX->clear();                         
    inputY->clear();                         
    lcsLabel->setText("LCS will appear here"); 
    dpTable->clear();                       
    dpTable->setRowCount(0);                 
    dpTable->setColumnCount(0);             
    dpMatrix.clear();                       
    directionMatrix.clear();                
    currentRow = 0;                         
    currentCol = 0;                         
    updateSequenceLabels();
}
