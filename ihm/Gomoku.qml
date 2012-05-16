 import QtQuick 1.0
 import "cell" 1.0

 Item {
     id: field
     property int clickx: 0
     property int clicky: 0

     width: 760; height: 760

     Grid {
         anchors.horizontalCenter: parent.horizontalCenter
         columns: cols; spacing: 0

         Repeater {
             id: repeater
             model: cells
             delegate: Cell {}
         }
     }
 }