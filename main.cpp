#include "http.cpp"
#include <ncurses.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

std::string welcome_screen(){
  initscr();
  start_color();
  use_default_colors();

  init_pair(1, COLOR_CYAN,   -1);
  init_pair(2, COLOR_GREEN,  -1);
  init_pair(3, COLOR_YELLOW, -1);
  init_pair(4, COLOR_RED,    -1);

  noecho();
  curs_set(1);
  raw();

  int win_hei = 7, win_wid = 40;
  int start_y = (LINES - win_hei) / 2;
  int start_x = (COLS  - win_wid) / 2;

  WINDOW* win = newwin(win_hei, win_wid, start_y, start_x);
  keypad(win, true);
  box(win, 0, 0);

  wattron(win, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(win, 0, (win_wid - 14) / 2, " MAYDAY ");
  wattroff(win, COLOR_PAIR(1) | A_BOLD);

  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, 2, 2, "Callsign:");
  wattroff(win, COLOR_PAIR(2));

  wattron(win, A_DIM);
  mvwprintw(win, 4, 2, "e.g. IGO7453, AIC101 (Caps Only)");
  mvwprintw(win, win_hei - 1, (win_wid - 16) / 2, " [Enter] search ");
  wattroff(win, A_DIM);

  wrefresh(win);

  std::string callsign;
  int ch;
  wmove(win, 2, 12);

  while(true){
    ch = wgetch(win);
    if(ch == '\n' || ch == KEY_ENTER){
      if(!callsign.empty()) break;
    } else if(ch == KEY_BACKSPACE || ch == 127){
      if(!callsign.empty()){
        callsign.pop_back();
        int cur_x = 12 + callsign.size();
        mvwprintw(win, 2, cur_x, " ");
        wmove(win, 2, cur_x);
        wrefresh(win);
      }
    } else if(isprint(ch) && callsign.size() < 10){
      callsign += toupper(ch);
      wattron(win, COLOR_PAIR(3));
      mvwprintw(win, 2, 12 + callsign.size() - 1, "%c", toupper(ch));
      wattroff(win, COLOR_PAIR(3));
      wrefresh(win);
    }
  }

  delwin(win);
  return callsign;
}

int main(){
  std::string callsign = welcome_screen();

  clear();
  curs_set(0);
  attron(COLOR_PAIR(1));
  mvprintw(LINES/2, (COLS - 16)/2, "Fetching data...");
  attroff(COLOR_PAIR(1));
  refresh();

  Plane p;
  std::mutex p_mutex;
  std::atomic<bool> quit{false};
  std::atomic<bool> fetching{false};

  // background fetch thread
  std::thread fetcher([&](){
    while(!quit){
      fetching = true;
      {
        std::lock_guard<std::mutex> lock(p_mutex);
        p.getdata(callsign);
      }
      fetching = false;
      // wait 5s between fetches, but check quit every 100ms
      for(int i = 0; i < 50 && !quit; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  int win_hei = 15, win_wid = 36;
  int start_y = (LINES - win_hei) / 2;
  int start_x = (COLS  - win_wid) / 2;

  WINDOW* win = newwin(win_hei, win_wid, start_y, start_x);
  keypad(win, true);
  wtimeout(win, 200); // non-blocking: returns ERR after 200ms

  auto draw = [&](){
    std::lock_guard<std::mutex> lock(p_mutex);
    werase(win);
    box(win, 0, 0);

    wattron(win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(win, 0, (win_wid - 12) / 2, " MAYDAY ");
    wattroff(win, COLOR_PAIR(1) | A_BOLD);

    // fetching indicator top-right
    if(fetching){
      wattron(win, COLOR_PAIR(1) | A_DIM);
      mvwprintw(win, 0, win_wid - 5, "~rx~");
      wattroff(win, COLOR_PAIR(1) | A_DIM);
    }

    if(!p.is_live){
      wattron(win, COLOR_PAIR(4));
      mvwprintw(win, win_hei / 2, (win_wid - 12) / 2, "No live data");
      wattroff(win, COLOR_PAIR(4));
      wrefresh(win);
      return;
    }

    auto row = [&](int y, const char* label, auto fmt, auto val){
      wattron(win, COLOR_PAIR(2));
      mvwprintw(win, y, 2, "%s", label);
      wattroff(win, COLOR_PAIR(2));
      wattron(win, COLOR_PAIR(3));
      mvwprintw(win, y, 10, fmt, val);
      wattroff(win, COLOR_PAIR(3));
    };

    row(2,  "FLT :", "%s", p.flight.c_str());
    row(3,  "CAT :", "%s", p.category.c_str());
    row(4,  "TYPE:", "%s", p.type.c_str());
    row(5,  "TAIL:", "%s", p.tailnum.c_str());

    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, 6, 2, "EMG :");
    wattroff(win, COLOR_PAIR(2));
    int emg_color = (p.emergency != "none" && !p.emergency.empty()) ? 4 : 3;
    wattron(win, COLOR_PAIR(emg_color));
    mvwprintw(win, 6, 10, "%s", p.emergency.c_str());
    wattroff(win, COLOR_PAIR(emg_color));

    row(7,  "ALT :", "%.0f ft",  p.alt);
    row(8,  "SPD :", "%.1f kt",  p.speed);
    row(9,  "MACH:", "%.3f",     p.mach);
    row(10, "LAT :", "%.4f",     p.lat);
    row(11, "LON :", "%.4f",     p.lon);
    row(12, "VS  :", "%.0f fpm", p.vspeed);

    wattron(win, A_DIM);
    mvwprintw(win, win_hei - 1, (win_wid - 10) / 2, " [q] quit ");
    wattroff(win, A_DIM);

    wrefresh(win);
  };

  draw();

  int ch;
  while(true){
    ch = wgetch(win);
    if(ch == 'q'){ quit = true; break; }
    draw();
  }

  fetcher.join();
  endwin();
  return 0;
}