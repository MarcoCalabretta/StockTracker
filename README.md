# StockTracker
Made by Marco Calabretta on August 18, 2023
Intended to be a (loose) clone of SimplyWallSt's Portfolio Summary feature

This allows users to input transactions they had with various publicly traded stocks or ETFs, then summarizes their portfolios in several csv files. There is one csv file for each stock (each stock is identified by its ticker, e.g. "msft"), and one csv file for the whole portfolio. Each file lists all transactions and dividends related to the stock (the portfolio wide file lists it for all stocks), along with a cagr and a total portfolio value (these are only calculated when the user uses the cagr feature). Additionally, there is a csv file that details how many shares of each stock the user owns, and a price for each that gets automatically updated when the user calculates their cagr.

NOTE: There are some known bugs in this app.

1. Sometimes stocks can be sold that shouldn't be able to be sold. For example, if I buy 10 shares of abc in 2012, then sell 8 shares in 2014, then bought another 10 in 2015, the program will let you sell 6 shares in 2013, because it doesn't realize that in a future transaction it will lead to you having negative shares. Feel free to fix the transaction_execute function in transactioncounter.c.

2. I get dividend and price data off the internet, from two sites. There are likely many publicly traded securities that don't work with the specific pattern matching I did for the urls in dividenddownload.c and the get_price function in stockcounter.c.

I hope you enjoy, I'll totally look at any suggestions you push, and probably implement them if they're well thought out
