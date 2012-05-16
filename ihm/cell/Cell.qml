import QtQuick 1.0

Item {
    id: flipable
    property int angle: 0
    
    width: 40;  height: 40
 	
    Image {
        anchors.centerIn: parent
        source: modelData.source
    }
    
     MouseArea {
         anchors.fill: parent
         acceptedButtons: Qt.LeftButton | Qt.RightButton
         onClicked: {
	 	    if (mouse.button == Qt.RightButton)
		       	rclick(index)
		    else
			clic(index)
         }
     }
}

