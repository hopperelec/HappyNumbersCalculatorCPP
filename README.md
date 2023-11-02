Calculates whether a number is a [happy number](https://en.wikipedia.org/wiki/Happy_number) as quickly as possible using several techniques including:
- Caching
  - Including recognising permutations of the same digits
- Multi-threading (CPU, not GPU)
  - Including a function to help choose an optimal number of threads to use
- Branch prediction

Default functionality is to time how many milliseconds it takes to cache the happiness of 2,000,000,000 numbers in base 10, outputting every 10,000,000th number, skipping permutations but using a single thread

Made alongside [AzureAqua](https://github.com/AzureAqua) for competitive programming