# WaybackFullTextSearch++
Work in progress software for indexing entire archives of sites (or sections of sites) from web.archive.org using manticore search.

## TODO
- [X] Download available page URLs from wayback API and store in sqlite file
  - [X] URL sqlite file handling class
  - [X] cURL class
  - [X] Parse downloaded API results into db file
- [X] Download pages from sqlite url DB into manticore instance
  - [X] cURL class
  - [X] Add functionality to sqlite class to get filtered results from db file 
  - [X] Manticore connection class
    - [X] Add required functions to cURL helper class
  - [X] Page parsing (get readable text for parsing)
  - [X] Insert parsed page data into manticore DB
  - [X] Batch operation page scraping
  - [X] Multithreaded page scraping
    - [ ] Improve multithreading
- [~] Search functionality
  - [X] Expand manticore class to have search features
  - [ ] Output results nicely
  - [ ] Web
    - [ ] Display results in web browser
    - [ ] Fully interactive search through web page
  - [ ] Advanced search features
  - [ ] Figure out how to combine proper pagination and grouping together multiple timestamps of the same page nicely
- [X] Option to route through TOR
- [ ] Write good documentation
- [ ] Fix *all* the function calls where returned errors arent handled
- [ ] Detection +exclusion of binary files (sometimes wayback mimetype is wrong)
  - [ ] Or alternatively, handle various binary file types properly (parse them)
- [X] Better command line options/handling
