

#include "../JuceLibraryCode/JuceHeader.h"
#include <string>

class MainContentComponent   :  public AudioAppComponent,
                                public ChangeListener,
                                public Button::Listener

{
public:
    //==============================================================================
    MainContentComponent():state(Stopped)
    {
        addAndMakeVisible(titleLabel);
        titleLabel.setText("DJ Norm Music Library", dontSendNotification);
        titleLabel.setFont(Font(24.0f,Font::bold));
        titleLabel.setColour(Label::textColourId,Colours::limegreen);
        titleLabel.setJustificationType(Justification::horizontallyCentred);
        
        //table.getHeader().addRow("Song Title");
        
        
        addAndMakeVisible(&openButton);
        openButton.setButtonText("Open");
        openButton.addListener(this);
        openButton.setColour(TextButton::buttonColourId, Colours::goldenrod);
       
        addAndMakeVisible(&playButton);
        playButton.setButtonText("Play");
        playButton.addListener(this);
        playButton.setColour(TextButton::buttonColourId, Colours::limegreen);
        playButton.setEnabled(false);
        
        addAndMakeVisible(&stopButton);
        stopButton.setButtonText("Stop");
        stopButton.addListener(this);
        stopButton.setColour(TextButton::buttonColourId, Colours::indianred);
        stopButton.setEnabled(false);
    
        //Initialize window settings
        setSize (650, 600);
        
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
        
       

        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if(readerSource == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        transportSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        transportSource.releaseResources();
    }

    void resized() override
    {
        titleLabel.setBounds(10,5,getWidth()-20, 30);
        openButton.setBounds (50, 35, 100, 25);
        playButton.setBounds (250, 35, 100, 25);
        stopButton.setBounds (450, 35, 100, 25);

    }
    
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else if ((state == Stopping) || (state == Playing))
                changeState (Stopped);
            else if (state == Pausing)
                changeState (Paused);
        }
    }
    
    void buttonClicked(Button* button) override
    {
        if(button == &openButton) openButtonClicked();
        if(button == &playButton) playButtonClicked();
        if(button == &stopButton) stopButtonClicked();
    }
    

private:
    //=============================================================================
    enum TransportState{
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };
    void changeState(TransportState newState){
        if (state != newState)
        {
            state = newState;
            
            switch (state)
            {
                case Stopped:
                    playButton.setButtonText ("Play");
                    stopButton.setButtonText ("Stop");
                    stopButton.setEnabled (false);
                    transportSource.setPosition (0.0);
                    break;
                    
                case Starting:
                    transportSource.start();
                    break;
                    
                case Playing:
                    playButton.setButtonText ("Pause");
                    stopButton.setButtonText ("Stop");
                    stopButton.setEnabled (true);
                    break;
                    
                case Pausing:
                    transportSource.stop();
                    break;
                    
                case Paused:
                    playButton.setButtonText ("Resume");
                    stopButton.setButtonText ("Reset");
                    break;
                    
                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }
    void openButtonClicked()
    {
        FileChooser chooser ("Select a Wave file to play...",
                             File::nonexistent,
                             "*.wav");
        
        if (chooser.browseForFileToOpen())
        {
            File file (chooser.getResult());
            AudioFormatReader* reader = formatManager.createReaderFor (file);
            
            if (reader != nullptr)
            {
                ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true);
                transportSource.setSource (newSource, 0, nullptr, reader->sampleRate);
                playButton.setEnabled (true);
                readerSource = newSource.release();
                //songTitleLabel.setText(.getText,dontSendNotification);
            }
        }
    }
    
    void playButtonClicked()
    {
        if(state == Stopped|| state == Paused)
        {
            changeState(Starting);
        }
        else if(state == Playing)
        {
            changeState(Pausing);
        }
    }
    
    void stopButtonClicked()
    {
        if(state == Paused)
        {
            changeState(Stopped);
        }
        else
        {
            changeState(Stopping);
        }
    }
    
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    Label titleLabel;
 
    AudioFormatManager formatManager;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


class TableDemoComponent    : public Component,
public TableListBoxModel
{
public:
    TableDemoComponent()   : font (14.0f)
    {
        // Load some data from an embedded XML file..
        loadData();
        
        // Create our table component and add it to this component..
        addAndMakeVisible (table);
        table.setModel (this);
        
        // give it a border
        table.setColour (ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness (1);
        
        // Add some columns to the table header, based on the column list in our database..
        forEachXmlChildElement (*columnList, columnXml)
        {
            table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                         columnXml->getIntAttribute ("columnId"),
                                         columnXml->getIntAttribute ("width"),
                                         50, 400,
                                         TableHeaderComponent::defaultFlags);
        }
        
        // we could now change some initial settings..
        table.getHeader().setSortColumnId (1, true); // sort forwards by the ID column
        table.getHeader().setColumnVisible (7, false); // hide the "length" column until the user shows it
        
        // un-comment this line to have a go of stretch-to-fit mode
        // table.getHeader().setStretchToFitActive (true);
        
        table.setMultipleSelectionEnabled (true);
    }
    
    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows() override
    {
        return numRows;
    }
    
    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        const Colour alternateColour (getLookAndFeel().findColour (ListBox::backgroundColourId)
                                      .interpolatedWith (getLookAndFeel().findColour (ListBox::textColourId), 0.03f));
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (alternateColour);
    }
    
    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (getLookAndFeel().findColour (ListBox::textColourId));
        g.setFont (font);
        
        if (const XmlElement* rowElement = dataList->getChildElement (rowNumber))
        {
            const String text (rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));
            
            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }
        
        g.setColour (getLookAndFeel().findColour (ListBox::backgroundColourId));
        g.fillRect (width - 1, 0, 1, height);
    }
    
    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);
            
            table.updateContent();
        }
    }
    
    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        if (columnId == 1 || columnId == 7) // The ID and Length columns do not have a custom component
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }
        
        if (columnId == 5) // For the ratings column, we return the custom combobox component
        {
            RatingColumnCustomComponent* ratingsBox = static_cast<RatingColumnCustomComponent*> (existingComponentToUpdate);
            
            // If an existing component is being passed-in for updating, we'll re-use it, but
            // if not, we'll have to create one.
            if (ratingsBox == nullptr)
                ratingsBox = new RatingColumnCustomComponent (*this);
            
            ratingsBox->setRowAndColumn (rowNumber, columnId);
            return ratingsBox;
        }
        
        // The other columns are editable text columns, for which we use the custom Label component
        EditableTextCustomComponent* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        
        // same as above...
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent (*this);
        
        textLabel->setRowAndColumn (rowNumber, columnId);
        return textLabel;
    }
    
    // This is overloaded from TableListBoxModel, and should choose the best width for the specified
    // column.
    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 5)
            return 100; // (this is the ratings column, containing a custom combobox component)
        
        int widest = 32;
        
        // find the widest bit of text in this column..
        for (int i = getNumRows(); --i >= 0;)
        {
            if (const XmlElement* rowElement = dataList->getChildElement (i))
            {
                const String text (rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));
                
                widest = jmax (widest, font.getStringWidth (text));
            }
        }
        
        return widest + 8;
    }
    
    // A couple of quick methods to set and get cell values when the user changes them
    int getRating (const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Rating");
    }
    
    void setRating (const int rowNumber, const int newRating)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Rating", newRating);
    }
    
    String getText (const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute ( getAttributeNameForColumnId(columnNumber));
    }
    
    void setText (const int columnNumber, const int rowNumber, const String& newText)
    {
        const String& columnName = table.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }
    
    //==============================================================================
    void resized() override
    {
        // position our table with a gap around its edge
        table.setBoundsInset (BorderSize<int> (8));
    }
    
    
private:
    TableListBox table;     // the table component itself
    Font font;
    
    ScopedPointer<XmlElement> musicData;   // This is the XML document loaded from the embedded file "music.xml"
    XmlElement* columnList; // A pointer to the sub-node of demoData that contains the list of columns
    XmlElement* dataList;   // A pointer to the sub-node of demoData that contains the list of data rows
    int numRows;            // The number of rows of data we've got
    
    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent  : public Label
    {
    public:
        EditableTextCustomComponent (TableDemoComponent& td)  : owner (td)
        {
            // double click to edit the label text; single click handled below
            setEditable (false, true, false);
        }
        
        void mouseDown (const MouseEvent& event) override
        {
            // single click on the label should simply select the row
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);
            
            Label::mouseDown (event);
        }
        
        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }
        
        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), dontSendNotification);
        }
        
        void paint (Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            if (! dynamic_cast<LookAndFeel_V4*> (&lf))
                lf.setColour (textColourId, Colours::black);
            
            Label::paint (g);
        }
        
    private:
        TableDemoComponent& owner;
        int row, columnId;
        Colour textColour;
    };
    
    //==============================================================================
    // This is a custom component containing a combo box, which we're going to put inside
    // our table's "rating" column.
    class RatingColumnCustomComponent    : public Component,
    private ComboBox::Listener
    {
    public:
        RatingColumnCustomComponent (TableDemoComponent& td)  : owner (td)
        {
            // just put a combo box inside this component
            addAndMakeVisible (comboBox);
            comboBox.addItem ("fab", 1);
            comboBox.addItem ("groovy", 2);
            comboBox.addItem ("hep", 3);
            comboBox.addItem ("mad for it", 4);
            comboBox.addItem ("neat", 5);
            comboBox.addItem ("swingin", 6);
            comboBox.addItem ("wild", 7);
            
            // when the combo is changed, we'll get a callback.
            comboBox.addListener (this);
            comboBox.setWantsKeyboardFocus (false);
        }
        
        void resized() override
        {
            comboBox.setBoundsInset (BorderSize<int> (2));
        }
        
        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox.setSelectedId (owner.getRating (row), dontSendNotification);
        }
        
        void comboBoxChanged (ComboBox*) override
        {
            owner.setRating (row, comboBox.getSelectedId());
        }
        
    private:
        TableDemoComponent& owner;
        ComboBox comboBox;
        int row, columnId;
    };
    
    //==============================================================================
    // A comparator used to sort our data when the user clicks a column header
    class DemoDataSorter
    {
    public:
        DemoDataSorter (const String& attributeToSortBy, bool forwards)
        : attributeToSort (attributeToSortBy),
        direction (forwards ? 1 : -1)
        {
        }
        
        int compareElements (XmlElement* first, XmlElement* second) const
        {
            int result = first->getStringAttribute (attributeToSort)
            .compareNatural (second->getStringAttribute (attributeToSort));
            
            if (result == 0)
                result = first->getStringAttribute ("ID")
                .compareNatural (second->getStringAttribute ("ID"));
            
            return direction * result;
        }
        
    private:
        String attributeToSort;
        int direction;
    };
    
    //==============================================================================
    // this loads the embedded database XML file into memory
    void loadData()
    {
        XmlDocument myMusic (File("music.xml"));
        musicData = myMusic.getDocumentElement();

        dataList   = musicData->getChildByName ("DATA");
        columnList = musicData->getChildByName ("COLUMNS");
        
        numRows = dataList->getNumChildElements();
    }
    
    // (a utility method to search our XML for the attribute that matches a column ID)
    String getAttributeNameForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnList, columnXml)
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }
        
        return {};
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableDemoComponent)
};
