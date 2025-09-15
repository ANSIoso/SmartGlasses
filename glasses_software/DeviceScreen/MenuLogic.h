#ifndef MENULOGIC_H
#define MENULOGIC_H

class MenuLogic {
private:
  int icon_bitmaps_LEN;
  
public:
  int item_selected;
  int item_previous;
  int item_next;
  bool menu;
  
  // Constructor
  MenuLogic(int menu_length) {
    icon_bitmaps_LEN = menu_length;
    item_selected = 0;
    item_previous = 0;
    item_next = 0;
    menu = true;
    updateItems();
  }
  
  // Update previous and next items based on current selection
  void updateItems() {
    // Handle wraparound for selected item
    if(item_selected >= icon_bitmaps_LEN)
      item_selected = 0;
    if(item_selected < 0)
      item_selected = icon_bitmaps_LEN - 1;
      
    // Calculate previous item with wraparound
    item_previous = item_selected - 1;
    if(item_previous < 0)
      item_previous = icon_bitmaps_LEN - 1;
      
    // Calculate next item with wraparound
    item_next = item_selected + 1;
    if(item_next >= icon_bitmaps_LEN)
      item_next = 0;
  }
  
  // Go to next menu item
  void nextItem() {
    item_selected++;
    updateItems();
  }
  
  // Go to previous menu item
  void previousItem() {
    item_selected--;
    updateItems();
  }
  
  // Toggle menu visibility/state
  void toggleMenu() {
    menu = !menu;
  }
  
  // Get current selected item index
  int getSelectedItem() {
    return item_selected;
  }
  
  // Get previous item index
  int getPreviousItem() {
    return item_previous;
  }
  
  // Get next item index
  int getNextItem() {
    return item_next;
  }
  
  // Check if menu is active
  bool isMenuActive() {
    return menu;
  }
};

#endif