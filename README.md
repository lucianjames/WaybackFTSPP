# WaybackFullTextSearch++
Work in progress software for indexing entire archives of sites (or sections of sites) from web.archive.org using manticore search.

## TODO
- [X] Download available page URLs from wayback API and store in sqlite file
  - [X] URL sqlite file handling class
  - [X] cURL class
  - [X] Parse downloaded API results into db file
  - [ ] Better command line options
- [ ] Download pages from sqlite url DB into manticore instance
  - [X] cURL class
  - [X] Add functionality to sqlite class to get filtered results from db file 
  - [X] Manticore connection class
    - [X] Add required functions to cURL helper class
  - [X] Page parsing (get readable text for parsing)
  - [X] Insert parsed page data into manticore DB
  - [ ] Batch operation page scraping
  - [ ] Multithreaded page scraping
- [ ] Search functionality
  - [ ] Expand manticore class to have search features
  - [ ] Output results nicely
  - [ ] Advanced search features
- [X] Option to route through TOR
- [ ] Write good documentation
