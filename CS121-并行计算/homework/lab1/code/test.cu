#include "cuckoo_hash_table.cuh"
#include "utils.cuh"
#include <set>
#include <unordered_set>

const int TABLE_SIZE = 1 << 25;
const int NUM_KEYS = 100000;

// Test function declarations
bool test_insertion(CuckooHashTable &table, const std::vector<uint32_t> &keys);
bool test_lookup(CuckooHashTable &table, const std::vector<uint32_t> &keys);
bool test_false_lookups(CuckooHashTable &table);

int main()
{

    std::vector<uint32_t> keys;
    std::set<uint32_t> unique_keys;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, NUM_KEYS * 10);

    // Generate unique random keys
    while (keys.size() < NUM_KEYS)
    {
        uint32_t key = dis(gen);
        if (unique_keys.insert(key).second)
        {
            keys.push_back(key);
        }
    }


    std::unordered_set<int> hash_set;

    for (int i = 0; i < keys.size(); ++i) {
        int hash1_v = hash0(keys[i], TABLE_SIZE);
        int hash2_v = compute_hash(SEED2, keys[i], TABLE_SIZE);
        int hash3_v = compute_hash(SEED3, keys[i], TABLE_SIZE);

        hash_set.insert(hash1_v);
        hash_set.insert(hash2_v);
        hash_set.insert(hash3_v);
    }

    // bool all_present = true;
    // for (int i = 0; i < TABLE_SIZE; ++i) {
    //     if (hash_set.find(i) == hash_set.end()) {
    //         all_present = false;
    //         std::cout << "Missing hash value: " << i << std::endl;
    //     }
    // }

    // if (all_present) {
    //     std::cout << "The set contains all values from [0, " << TABLE_SIZE << ")." << std::endl;
    // } else {
    //     std::cout << "The set does not contain all values from [0, " << TABLE_SIZE << ")." << std::endl;
    // }

    // for (int i = 0; i < NUM_KEYS; i++)
    // {
    //     keys.push_back(i + 1);
    // }

    // std::cout << "Random keys are"  << std::endl;
    // for (auto key : keys) std::cout << key << " ";
    // std::cout << std::endl;

    // Create hash table
    // CuckooHashTable table(TABLE_SIZE, clog2(NUM_KEYS), false);
    CuckooHashTable table(TABLE_SIZE, 1000000, false);


    // Run tests
    std::cout << "Running tests...\n\n";

    // Test 1: Insertion
    std::cout << "Test 1 - Insertion: ";
    if (test_insertion(table, keys))
    {
        std::cout << "PASSED\n";
    }

    // Test 2: Lookup of existing keys
    std::cout << "Test 2 - Lookup existing keys: ";
    if (test_lookup(table, keys))
    {
        std::cout << "PASSED\n";
    }

    // Test 3: Lookup of non-existing keys
    std::cout << "Test 3 - Lookup non-existing keys: ";
    if (test_false_lookups(table))
    {
        std::cout << "PASSED\n";
    }

    return 0;
}

bool test_insertion(CuckooHashTable &table, const std::vector<uint32_t> &keys)
{
    table.insert(keys);

    table.display(false);

    // Verify insertion through lookup
    std::vector<int> results = table.lookup(keys);
    int found = 0;
    for (int i = 0; i < keys.size(); i++)
    {
        if (results[i])
        {
            found++;
        }
        else {
            std::cout << "Key " << keys[i] << " not found" << std::endl;
            std::cout << hash0(keys[i], TABLE_SIZE) << " " << compute_hash(SEED2, keys[i], TABLE_SIZE) << " " << compute_hash(SEED3, keys[i], TABLE_SIZE) << std::endl;

        }
    }


    if (found != keys.size())
    {
        std::cout << "FAILED\n";
        std::cout << "Error: Only " << found << " out of " << keys.size()
                  << " keys were successfully inserted\n";
        std::cout << "Current table state:\n";
        table.display(false);
        return false;
    }
    return true;
}

bool test_lookup(CuckooHashTable &table, const std::vector<uint32_t> &keys)
{
    std::vector<int> results = table.lookup(keys);

    for (size_t i = 0; i < keys.size(); i++)
    {
        if (!results[i])
        {
            std::cout << "FAILED\n";
            std::cout << "Error: Could not find key " << keys[i]
                      << " that was previously inserted\n";

            std::cout << "Lookup Results is" << std::endl;
            for (auto result : results)
                std::cout << result << " ";
            std::cout << std::endl;
            return false;
        }
    }
    return true;
}

bool test_false_lookups(CuckooHashTable &table)
{
    // Test keys that definitely weren't inserted
    std::vector<uint32_t> non_existing_keys;
    for (int i = 0; i < 10; i++)
    {
        non_existing_keys.push_back(NUM_KEYS * 10 + i);
    }

    std::vector<int> results = table.lookup(non_existing_keys);
    int false_positives = 0;

    for (size_t i = 0; i < results.size(); i++)
    {
        if (results[i])
        {
            std::cout << "FAILED\n";
            std::cout << "Error: False positive detected for key "
                      << non_existing_keys[i] << "\n";
            false_positives++;
        }
    }

    if (false_positives > 0)
    {
        std::cout << "Total false positives: " << false_positives
                  << " out of " << non_existing_keys.size() << "\n";
        return false;
    }
    return true;
}