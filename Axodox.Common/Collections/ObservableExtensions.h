#pragma once
#include "pch.h"

namespace Axodox::Collections
{
  template<typename TSourceCollection, typename TTargetCollection, typename TMapper>
  void update_observable_collection(const TSourceCollection& source, TTargetCollection& target, TMapper mapper)
  {
    //Get current ID list
    std::set<typename TMapper::id_t> itemsToDelete;
    for (const auto& item : target)
    {
      itemsToDelete.emplace(mapper.get_id(item));
    }

    //Upsert items
    for (const auto& item : source)
    {
      itemsToDelete.erase(mapper.get_id(item));

      //Remove item from old location on order change
      {
        auto itBegin = begin(target);
        auto itCurrent = itBegin;
        auto itEnd = end(target);

        while (itCurrent != itEnd)
        {
          if (mapper.get_id(*itCurrent) == mapper.get_id(item) &&
            mapper.compare(*itCurrent, item) != 0)
          {
            target.RemoveAt(uint32_t(itCurrent - itBegin));
            break;
          }

          itCurrent++;
        }
      }

      //Add or update item
      {
        auto itBegin = begin(target);
        auto itCurrent = itBegin;
        auto itEnd = end(target);

        while (itCurrent != itEnd && mapper.compare(*itCurrent, item) < 0)
        {
          itCurrent++;
        }

        auto mapped = mapper.map(item);
        if (itCurrent == itEnd)
        {
          target.Append(mapped);
        }
        else if (mapper.get_id(*itCurrent) == mapper.get_id(item))
        {
          auto index = uint32_t(itCurrent - itBegin);
          if (target.GetAt(index) != mapped)
          {
            target.SetAt(index, mapped);
          }
        }
        else
        {
          target.InsertAt(uint32_t(itCurrent - itBegin), mapped);
        }
      }
    }

    //Remove old items
    {
      auto itBegin = begin(target);
      auto itCurrent = itBegin;
      auto itEnd = end(target);
      while (itCurrent != itEnd)
      {
        if (itemsToDelete.count(mapper.get_id(*itCurrent)))
        {
          target.RemoveAt(uint32_t(itCurrent - itBegin));
          itEnd = end(target);
        }
        else
        {
          itCurrent++;
        }
      }
    }
  }
}