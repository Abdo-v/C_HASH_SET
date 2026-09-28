#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>


template <typename Key, size_t N = 5>
class ADS_set {
public:

  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;

  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing
//_____________________IMP_________________________
private:

  struct bucket{
    Key elements[N];
    unsigned local_depth {};
    unsigned size = 0;
    /*Key& operator[](size_type i){
      return elements[i];
    }*/

    bool insert(const key_type& key){
      if(size < N){
        elements[size++] = key;
        return true;
      }
      else{return false;}  
    }

    bool has(const key_type& key){
      key_equal eq;
        for(size_type i =0; i < size; ++i){
            if(eq(key, elements[i])){return true;}
        }
        return false;
    }

    bool full(){
      if(size == N){return true;}
      else{return false;}
    }
  };
  unsigned sz{};
  unsigned global_depth{}; 
  unsigned bucket_size = N;
  bucket** directory;
//_________________private methods_________________
  size_type hash_max() const{
    if(global_depth == 0){return 1;}
    /*size_type res = 1;
    for(unsigned i = 0; i<global_depth; ++i){
      res = res * 2;
    }*/
    return 1 << global_depth;
    //return res;
  }
  size_type h(const key_type& key) const{return hasher{}(key)% hash_max();}

  void split_bucket(const Key& key){
    size_type hv = h(key);
    bucket* old = directory[hv];
    if(directory[hv]->size != bucket_size){return;}
    if(directory[hv]->local_depth == global_depth){
      this->expand();
    }
    bucket* buc = new bucket;
    directory[hv]->local_depth++;
    buc->local_depth = directory[hv]->local_depth;
    buc->size =0;
    Key bo[N];
    directory[hv]->size =0;

    for (unsigned i = 0; i < N; ++i) {
      bo[i] = directory[hv]->elements[i];
    }
    bool flag{true};
    for(size_type i = 0; i < hash_max() ; ++i){
      if(old->elements == directory[i]->elements && flag){
        flag = false; continue;
      }
      if(old->elements == directory[i]->elements && !flag){
        flag = true;
        directory[i] = buc;
      }
    }
    for(size_type e = 0; e <N; ++e){
      add(bo[e]);
    }
  }

  void expand(){
    bucket** dir = new bucket*[2*hash_max()]; 
    for(size_t  i = 0; i < hash_max(); ++i){
      dir[i] = directory[i];
    }
    for(size_t  i = hash_max(); i < 2*hash_max(); ++i){
        dir[i] = directory[i-hash_max()];
    }
    delete[] directory;

    directory = dir;
    global_depth++;
  }
  bool add(const Key& k){
    size_type hv = h(k);
    if(directory[hv]->has(k)){return false;}

    if(directory[hv]->full()){
        split_bucket(k);
        return add(k);        
    }
 
    return directory[hv]->insert(k);  
  }
//___________________M&C_______________________
public:
  ADS_set(): sz{0}, global_depth{0}, bucket_size{N} {
    directory = new bucket*[1];
    directory[0] = new bucket;
    directory[0]->local_depth = global_depth;
    
  }                                // PH1
  ADS_set(std::initializer_list<key_type> ilist) : sz{0}, global_depth{2}, bucket_size{N} {
    directory = new bucket*[4];
    for (size_type i = 0; i < hash_max(); ++i) {
      directory[i] = new bucket;
      directory[i]->local_depth = global_depth;
    }
    for (const auto& v : ilist) {
        if(add(v)){++sz;}
    }
} // PH1

  template<typename InputIt> ADS_set(InputIt first, InputIt last): sz{0}, global_depth{2}, bucket_size{N}{
    directory = new bucket*[4];
    for (size_type i = 0; i < hash_max(); ++i) {
      directory[i] = new bucket;
      directory[i]->local_depth = global_depth;
    }
    for(auto i = first; i != last; ++i){
      if(add(*i)){++sz;}
    }
  }     // PH1

  ~ADS_set() {
    //std::cout << "DESTRUCTOR CALLED!!!" << std::endl;
    if(hash_max() == 1){
      delete directory[0];
      delete[] directory;
    }
    else{
    bool* arr = new bool[hash_max()];
    for(size_type i=0; i < hash_max(); ++i){arr[i] = true;}
    for(size_type e =0; e < hash_max()-1; ++e){
      for(size_type e2 =e+1; e2 < hash_max(); ++e2){
        if(directory[e] == directory[e2]){arr[e2] = false;}
      }
    }
    for (size_type i = 0; i < hash_max(); ++i) {
      if(arr[i]){delete directory[i];}
    }
    delete[] directory;
    delete[] arr;
    //std::cout << "DESTRUCTOR success!!!" << std::endl;
    } 
  }    // PH1

  size_type size() const{return sz;}// PH1
  bool empty() const{return size()==0;}// PH1

  void insert(std::initializer_list<key_type> ilist){
    for(const auto& v : ilist){
      if(add(v)){++sz;}
    }
  }                  // PH1

  template<typename InputIt> void insert(InputIt first, InputIt last){
    for(auto i = first; i != last; ++i){
      if(add(*i)){++sz;}
    }
  }// PH1

  size_type count(const key_type &key) const{
    size_type hash_val = h(key);
    if(directory[hash_val]->size == 0){
      return 0;
    }
    /*for (size_type e =0; e < directory[hash_val]->size; ++e) {
      if (fuk(key, directory[hash_val]->elements[e])) {
        //std::cout << "FOUND: " << e->val << std::endl;
        return 1;
      }
    }*/
    return directory[hash_val]->has(key); 
  }                          // PH1

  void dump(std::ostream &o = std::cerr) const{
    o << "____________________________________________________________" << std::endl;
    o << "NUMBER OF ELEMENTS: " << sz << std::endl;
    o << "tablesize: " << hash_max() << ", bucket_size: " << N << ", global_depth: " << global_depth << std::endl;
    for(size_t d =0; d < hash_max(); ++d){
      o << "Bucket: " << d << " --> "; 
      bool p = false;
      size_type i = 0;
      if(d!=0){
        for(size_type n = 0; n < d; ++n){
            if(directory[d]->elements == directory[n]->elements){p = true; i = n; break;}
        }
      }
      if(d == 0 || !p){
        for(size_type e =0; e < bucket_size ; ++e){
          if(e < directory[d]->size){
            o << directory[d]->elements[e] << ", ";
          }
          else{o << "[], ";}
        }
      }
      if(p){ o << "Bucket " << i;}
      o  << std::endl;
    }
    o << "____________________________________________________________" << std::endl;
  }

};

#endif