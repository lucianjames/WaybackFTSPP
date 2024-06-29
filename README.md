# WaybackFullTextSearch++
Work in progress software for indexing entire archives of sites (or sections of sites) from web.archive.org using manticore search.

## TODO
- [X] Download available page URLs from wayback API and store in sqlite file
  - [X] URL sqlite file handling class
  - [X] cURL class
  - [ ] Better command line options
- [ ] Download pages from sqlite url DB into manticore instance
  - [X] cURL class  
  - [ ] Manticore connection class
  - [ ] Page parsing (get readable text for parsing)
  - [ ] Insert parsed page data into manticore DB
- [ ] Search functionality
  - [ ] Expand manticore class to have search features
  - [ ] Output results nicely
  - [ ] Advanced search features
