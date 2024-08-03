# WaybackFullTextSearch++
Work in progress software for indexing entire archives of sites (or sections of sites) from web.archive.org using manticore search.

## Usage
Very very quick overview, more details fully documenting will be added later
### Step 1: Create sqlite DB file of url+timestamp combinations to scrape from archive.org
`./WaybackGetUrls -o dbfilename.sqlite -d example.com`
```
Usage:
  WaybackGetUrls [OPTION...]

  -o, --output_file arg  Sqlite3 DB name
  -d, --domain arg       Search query
      --tor              Route requests through TOR
      --tor-port arg     Port to run TOR proxy on (default: 9051)
  -h, --help             Print usage
```
### Step 2: Scrape pages from archive.org
This requires you have an instance of manticore search running on your machine
`./WaybackScrapePages -f dbfilename.sqlite -t manticoreTableName`
```
Usage:
  WaybackScrapePages [OPTION...]

  -t, --table arg     Manticore table name
  -f, --db-file arg   File containing URLs to scrape
      --tor           Route requests through TOR
      --tor-port arg  Port to run TOR proxy on (default: 9051)
  -h, --help          Print usage
```
### Step 3: Search scraped and parsed data
`/WaybackSeachTable -t manticoreTableName -q searchQuery`
```
Usage:
  WaybackSearchTable [OPTION...]

  -t, --table arg       Manticore table name
  -q, --query arg       Search query
  -n, --results arg     Number of results (default: 10)
  -p, --page arg        Page number, starts at 0 (default: 0)
  -s, --server-url arg  Manticore database URL (<ip>:<port>) (default:
                        127.0.0.1:9308)
  -h, --help            Print usage
```

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
